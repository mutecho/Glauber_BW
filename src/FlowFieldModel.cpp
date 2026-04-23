#include "blastwave/FlowFieldModel.h"

#include <algorithm>
#include <cmath>
#include <limits>

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
  blastwave::FlowFieldSample evaluateCovarianceEllipseFlow(const blastwave::FlowFieldContext &context,
                                                           double x,
                                                           double y,
                                                           const blastwave::FlowFieldParameters &parameters) {
    blastwave::FlowFieldSample sample;
    const EllipseMetricSample metric = sampleEllipseMetric(context.ellipse, x, y);
    if (!metric.valid) {
      return sample;
    }

    sample.rTilde = metric.rTilde;
    sample.phiB = std::atan2(metric.qMinor, metric.qMajor);
    sample.rhoRaw = std::pow(sample.rTilde, parameters.flowPower) * (parameters.rho0 + parameters.rho2 * context.ellipse.eps2 * std::cos(2.0 * sample.phiB));
    sample.betaT = computeBetaT(sample.rhoRaw);
    sample.betaX = sample.betaT * metric.normalX;
    sample.betaY = sample.betaT * metric.normalY;
    return sample;
  }

  // Sample the transverse velocity from the density-gradient normal while
  // falling back to the inverse-covariance normal in flat or numerically
  // degenerate regions.
  blastwave::FlowFieldSample evaluateDensityNormalFlow(const blastwave::FlowFieldContext &context,
                                                       double x,
                                                       double y,
                                                       const blastwave::FlowFieldParameters &parameters) {
    blastwave::FlowFieldSample sample;
    const EllipseMetricSample metric = sampleEllipseMetric(context.ellipse, x, y);
    if (!metric.valid) {
      return sample;
    }

    const blastwave::FlowDensitySample densitySample = blastwave::evaluateDensityField(context, x, y);
    const double gradientMagnitude = std::hypot(densitySample.gradientX, densitySample.gradientY);
    const double flatness = gradientMagnitude * context.flowDensitySigma / std::max(densitySample.density, kDensityEpsilon);

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
    sample.phiB = std::atan2(normalY, normalX);
    sample.rhoRaw = parameters.rho0 * std::pow(sample.rTilde, parameters.flowPower);
    sample.betaT = computeBetaT(sample.rhoRaw);
    sample.betaX = sample.betaT * normalX;
    sample.betaY = sample.betaT * normalY;
    return sample;
  }

}  // namespace

namespace blastwave {

  FlowFieldSample evaluateFlowField(const FlowFieldContext &context, double x, double y, const FlowFieldParameters &parameters) {
    switch (parameters.velocitySamplerMode) {
      case FlowVelocitySamplerMode::CovarianceEllipse:
        return evaluateCovarianceEllipseFlow(context, x, y, parameters);
      case FlowVelocitySamplerMode::DensityNormal:
        return evaluateDensityNormalFlow(context, x, y, parameters);
    }

    return {};
  }

}  // namespace blastwave
