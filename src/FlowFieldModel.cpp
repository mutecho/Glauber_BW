#include "blastwave/FlowFieldModel.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "blastwave/EmissionSampler.h"
#include "blastwave/EventMedium.h"

namespace {

  constexpr double kBetaTMax = 0.95;
  constexpr double kDirectionTolerance = 1.0e-12;
  constexpr double kFlatRegionTolerance = 1.0e-6;
  constexpr double kDensityEpsilon = 1.0e-18;

  struct EllipseMetricSample {
    bool valid = false;
    double deltaX = 0.0;
    double deltaY = 0.0;
    double normalX = 0.0;
    double normalY = 0.0;
    double qX = 0.0;
    double qY = 0.0;
    double qMajor = 0.0;
    double qMinor = 0.0;
    double rTilde = 0.0;
  };

  double computeBetaT(double rhoRaw) {
    return std::min(kBetaTMax, std::tanh(std::max(0.0, rhoRaw)));
  }

  EllipseMetricSample sampleEllipseMetric(const blastwave::FlowEllipseInfo &ellipse, double x, double y) {
    EllipseMetricSample metric;
    if (!ellipse.valid) {
      return metric;
    }

    metric.deltaX = x - ellipse.centerX;
    metric.deltaY = y - ellipse.centerY;
    metric.qX = ellipse.inverseSigmaXX * metric.deltaX + ellipse.inverseSigmaXY * metric.deltaY;
    metric.qY = ellipse.inverseSigmaXY * metric.deltaX + ellipse.inverseSigmaYY * metric.deltaY;
    metric.qMajor = metric.qX * ellipse.majorAxisX + metric.qY * ellipse.majorAxisY;
    metric.qMinor = metric.qX * ellipse.minorAxisX + metric.qY * ellipse.minorAxisY;

    const double qNorm = std::hypot(metric.qX, metric.qY);
    if (!std::isfinite(qNorm) || qNorm <= kDirectionTolerance) {
      return metric;
    }

    const double rTilde2 = metric.deltaX * metric.qX + metric.deltaY * metric.qY;
    if (!std::isfinite(rTilde2) || rTilde2 < 0.0) {
      return metric;
    }

    metric.normalX = metric.qX / qNorm;
    metric.normalY = metric.qY / qNorm;
    metric.rTilde = std::sqrt(std::max(0.0, rTilde2));
    metric.valid = true;
    return metric;
  }

  // Use the historical ellipse-normal sampler as one concrete backend of the
  // generalized fluid-element velocity sampling interface.
  blastwave::FlowFieldSample evaluateCovarianceEllipseFlow(const blastwave::EventMedium &medium, double x, double y, const blastwave::FlowFieldParameters &parameters) {
    blastwave::FlowFieldSample sample;
    const EllipseMetricSample metric = sampleEllipseMetric(medium.emissionGeometry, x, y);
    if (!metric.valid) {
      return sample;
    }

    sample.rTilde = metric.rTilde;
    const double phiBLab = std::atan2(metric.normalY, metric.normalX);
    // The public knob is kappa2 = a2 / eps2_initial. Keep the response tied to
    // the initial eccentricity vector even when the emission geometry evolves.
    const double a2 = parameters.kappa2 * medium.participantGeometry.eps2;
    if (medium.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse) {
      const double modulation = std::exp(2.0 * a2 * std::cos(2.0 * (phiBLab - medium.participantGeometry.psi2)));
      sample.phiB = phiBLab;
      sample.rhoRaw = parameters.rho0 * std::pow(sample.rTilde, parameters.flowPower) * modulation;
    } else {
      sample.phiB = std::atan2(metric.qMinor, metric.qMajor);
      sample.rhoRaw = std::pow(sample.rTilde, parameters.flowPower) * (parameters.rho0 + a2 * std::cos(2.0 * sample.phiB));
    }
    sample.betaT = computeBetaT(sample.rhoRaw);
    sample.betaX = sample.betaT * metric.normalX;
    sample.betaY = sample.betaT * metric.normalY;
    return sample;
  }

  // Sample the transverse velocity from the density-gradient normal while
  // falling back to the inverse-covariance normal in flat or numerically
  // degenerate regions.
  blastwave::FlowFieldSample evaluateDensityNormalFlow(const blastwave::EventMedium &medium, double x, double y, const blastwave::FlowFieldParameters &parameters) {
    blastwave::FlowFieldSample sample;
    const EllipseMetricSample metric = sampleEllipseMetric(medium.emissionGeometry, x, y);
    if (!metric.valid) {
      return sample;
    }

    const blastwave::DensityFieldSample densitySample = blastwave::evaluateDensityField(medium.emissionDensity, x, y);
    const double gradientMagnitude = std::hypot(densitySample.gradientX, densitySample.gradientY);
    const double flatness = gradientMagnitude * medium.emissionDensity.gaussianSigma / std::max(densitySample.density, kDensityEpsilon);

    double normalX = 0.0;
    double normalY = 0.0;
    if (std::isfinite(gradientMagnitude) && gradientMagnitude > kDirectionTolerance && std::isfinite(flatness) && flatness >= kFlatRegionTolerance) {
      normalX = -densitySample.gradientX / gradientMagnitude;
      normalY = -densitySample.gradientY / gradientMagnitude;
    } else {
      normalX = metric.normalX;
      normalY = metric.normalY;
    }

    const double normalMagnitude = std::hypot(normalX, normalY);
    if (!std::isfinite(normalMagnitude) || normalMagnitude <= kDirectionTolerance) {
      return sample;
    }

    normalX /= normalMagnitude;
    normalY /= normalMagnitude;
    sample.rTilde = metric.rTilde;
    const double phiBLab = std::atan2(normalY, normalX);
    sample.phiB = phiBLab;
    if (medium.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse) {
      const double a2 = parameters.kappa2 * medium.participantGeometry.eps2;
      const double modulation = std::exp(2.0 * a2 * std::cos(2.0 * (phiBLab - medium.participantGeometry.psi2)));
      sample.rhoRaw = parameters.rho0 * std::pow(sample.rTilde, parameters.flowPower) * modulation;
    } else {
      sample.rhoRaw = parameters.rho0 * std::pow(sample.rTilde, parameters.flowPower);
    }
    sample.betaT = computeBetaT(sample.rhoRaw);
    sample.betaX = sample.betaT * normalX;
    sample.betaY = sample.betaT * normalY;
    return sample;
  }

  // Interpret sampler-provided V2 gradient-response beta values as the full
  // transverse flow state for this emission site.
  blastwave::FlowFieldSample evaluateGradientResponseFlow(const blastwave::EmissionSite &site) {
    blastwave::FlowFieldSample sample;
    sample.betaX = site.betaTX;
    sample.betaY = site.betaTY;
    sample.betaT = std::hypot(sample.betaX, sample.betaY);
    if (!std::isfinite(sample.betaT) || sample.betaT <= 0.0) {
      sample.betaX = 0.0;
      sample.betaY = 0.0;
      sample.betaT = 0.0;
      sample.phiB = 0.0;
      sample.rhoRaw = 0.0;
      return sample;
    }

    if (sample.betaT >= 1.0) {
      const double scale = std::nextafter(1.0, 0.0) / sample.betaT;
      sample.betaX *= scale;
      sample.betaY *= scale;
      sample.betaT = std::nextafter(1.0, 0.0);
    }
    sample.phiB = std::atan2(sample.betaY, sample.betaX);
    sample.rhoRaw = std::atanh(sample.betaT);
    return sample;
  }

}  // namespace

namespace blastwave {

  FlowFieldSample evaluateFlowField(const EventMedium &medium, double x, double y, const FlowFieldParameters &parameters) {
    switch (parameters.velocitySamplerMode) {
      case FlowVelocitySamplerMode::CovarianceEllipse:
        return evaluateCovarianceEllipseFlow(medium, x, y, parameters);
      case FlowVelocitySamplerMode::DensityNormal:
        return evaluateDensityNormalFlow(medium, x, y, parameters);
      case FlowVelocitySamplerMode::GradientResponse:
        return {};
    }

    return {};
  }

  FlowFieldSample evaluateFlowField(const EventMedium &medium, const EmissionSite &site, const FlowFieldParameters &parameters) {
    switch (parameters.velocitySamplerMode) {
      case FlowVelocitySamplerMode::CovarianceEllipse:
      case FlowVelocitySamplerMode::DensityNormal:
        return evaluateFlowField(medium, site.position.x, site.position.y, parameters);
      case FlowVelocitySamplerMode::GradientResponse:
        return evaluateGradientResponseFlow(site);
    }

    return {};
  }

}  // namespace blastwave
