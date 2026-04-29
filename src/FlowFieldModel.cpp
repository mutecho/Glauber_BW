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

  // Guard inverse hyperbolic tangent against tiny numerical overshoots.
  double safeAtanh(double value) {
    const double atanhLimit = std::nextafter(1.0, 0.0);
    return std::atanh(std::clamp(value, -atanhLimit, atanhLimit));
  }

  bool affineParametersAreValid(const blastwave::FlowFieldParameters &parameters) {
    return std::isfinite(parameters.affineDeltaTauRef) && parameters.affineDeltaTauRef > 0.0 && std::isfinite(parameters.affineKappaFlow)
           && std::isfinite(parameters.affineKappaAniso) && std::isfinite(parameters.affineUMax) && parameters.affineUMax > 0.0 && parameters.affineUMax < 1.0;
  }

  // Recover the shared V1a second-order response multiplier from the initial
  // participant eccentricity vector, independent of the evolved emission shape.
  double computeAffineKappaModulation(double phiBLab, const blastwave::EventMedium &medium, const blastwave::FlowFieldParameters &parameters) {
    const double a2 = parameters.kappa2 * medium.participantGeometry.eps2;
    return std::exp(2.0 * a2 * std::cos(2.0 * (phiBLab - medium.participantGeometry.psi2)));
  }

  // Map one event-level position to covariance-normal geometry metrics.
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

  // Sample the density-gradient normal with covariance-normal fallback.
  bool sampleDensityNormalDirection(const blastwave::EventMedium &medium, double x, double y, const EllipseMetricSample &metric, double &normalX, double &normalY) {
    if (!metric.valid) {
      return false;
    }

    const blastwave::DensityFieldSample densitySample = blastwave::evaluateDensityField(medium.emissionDensity, x, y);
    const double gradientMagnitude = std::hypot(densitySample.gradientX, densitySample.gradientY);
    const double flatness = gradientMagnitude * medium.emissionDensity.gaussianSigma / std::max(densitySample.density, kDensityEpsilon);

    if (std::isfinite(gradientMagnitude) && gradientMagnitude > kDirectionTolerance && std::isfinite(flatness) && flatness >= kFlatRegionTolerance) {
      normalX = -densitySample.gradientX / gradientMagnitude;
      normalY = -densitySample.gradientY / gradientMagnitude;
    } else {
      normalX = metric.normalX;
      normalY = metric.normalY;
    }

    const double normalMagnitude = std::hypot(normalX, normalY);
    if (!std::isfinite(normalMagnitude) || normalMagnitude <= kDirectionTolerance) {
      return false;
    }

    normalX /= normalMagnitude;
    normalY /= normalMagnitude;
    return true;
  }

  // Build shared affine-effective closure values used by both runtime modes.
  bool computeAffineEffectiveSharedTerms(const blastwave::EventMedium &medium,
                                         const blastwave::FlowFieldParameters &parameters,
                                         double &hInEff,
                                         double &hOutEff,
                                         double &hBar,
                                         double &rBar) {
    if (!medium.affineEffectiveClosure.valid || !affineParametersAreValid(parameters)) {
      return false;
    }

    hInEff = medium.affineEffectiveClosure.lambdaIn / parameters.affineDeltaTauRef;
    hOutEff = medium.affineEffectiveClosure.lambdaOut / parameters.affineDeltaTauRef;
    hBar = 0.5 * (hInEff + hOutEff);
    rBar = std::sqrt(medium.affineEffectiveClosure.sigmaInFinal * medium.affineEffectiveClosure.sigmaOutFinal);
    return std::isfinite(hInEff) && std::isfinite(hOutEff) && std::isfinite(hBar) && std::isfinite(rBar);
  }

  // Convert closure-plus-mode selection into event-level debug diagnostics.
  blastwave::AffineEffectiveFlowInfo computeAffineEffectiveFlowInfoImpl(const blastwave::EventMedium &medium, const blastwave::FlowFieldParameters &parameters) {
    blastwave::AffineEffectiveFlowInfo info;
    info.affineEffectiveMode = parameters.affineEffectiveMode;

    double hInEff = 0.0;
    double hOutEff = 0.0;
    double hBar = 0.0;
    double rBar = 0.0;
    if (!computeAffineEffectiveSharedTerms(medium, parameters, hInEff, hOutEff, hBar, rBar)) {
      return info;
    }

    info.hInEff = hInEff;
    info.hOutEff = hOutEff;
    info.affineUMax = parameters.affineUMax;

    if (parameters.affineEffectiveMode == blastwave::AffineEffectiveMode::AdditiveRho) {
      // Additive-rho mode keeps a rho0 baseline plus affine geometry correction.
      const double uGeomIso = parameters.affineKappaFlow * hBar * rBar;
      const double uGeomIn = parameters.affineKappaFlow * std::sqrt((hInEff * medium.affineEffectiveClosure.sigmaInFinal)
                                                                     * (hInEff * medium.affineEffectiveClosure.sigmaInFinal));
      const double uGeomOut = parameters.affineKappaFlow * std::sqrt((hOutEff * medium.affineEffectiveClosure.sigmaOutFinal)
                                                                      * (hOutEff * medium.affineEffectiveClosure.sigmaOutFinal));
      const double uGeomIsoClipped = std::min(uGeomIso, parameters.affineUMax);
      const double uGeomInClipped = std::min(uGeomIn, parameters.affineUMax);
      const double uGeomOutClipped = std::min(uGeomOut, parameters.affineUMax);

      info.surfaceRhoBase = parameters.rho0;
      info.surfaceRhoGeomIso = safeAtanh(uGeomIsoClipped);
      info.surfaceRhoGeomIn = safeAtanh(uGeomInClipped);
      info.surfaceRhoGeomOut = safeAtanh(uGeomOutClipped);
      info.surfaceRhoTotalIn = std::max(0.0, info.surfaceRhoBase + (info.surfaceRhoGeomIn - info.surfaceRhoGeomIso));
      info.surfaceRhoTotalOut = std::max(0.0, info.surfaceRhoBase + (info.surfaceRhoGeomOut - info.surfaceRhoGeomIso));
      info.surfaceBetaInRaw = std::tanh(info.surfaceRhoTotalIn);
      info.surfaceBetaOutRaw = std::tanh(info.surfaceRhoTotalOut);
      info.surfaceBetaInClipped = std::min(info.surfaceBetaInRaw, parameters.affineUMax);
      info.surfaceBetaOutClipped = std::min(info.surfaceBetaOutRaw, parameters.affineUMax);
    } else {
      // Full-tensor mode directly converts principal-axis expansion rates to velocity.
      info.surfaceBetaInRaw = std::abs(parameters.affineKappaFlow * hInEff * medium.affineEffectiveClosure.sigmaInFinal);
      info.surfaceBetaOutRaw = std::abs(parameters.affineKappaFlow * hOutEff * medium.affineEffectiveClosure.sigmaOutFinal);
      info.surfaceBetaInClipped = std::min(info.surfaceBetaInRaw, info.affineUMax);
      info.surfaceBetaOutClipped = std::min(info.surfaceBetaOutRaw, info.affineUMax);
    }

    const double fields[] = {info.hInEff,
                             info.hOutEff,
                             info.affineUMax,
                             info.surfaceBetaInRaw,
                             info.surfaceBetaOutRaw,
                             info.surfaceBetaInClipped,
                             info.surfaceBetaOutClipped,
                             info.surfaceRhoBase,
                             info.surfaceRhoGeomIso,
                             info.surfaceRhoGeomIn,
                             info.surfaceRhoGeomOut,
                             info.surfaceRhoTotalIn,
                             info.surfaceRhoTotalOut};
    for (double field : fields) {
      if (!std::isfinite(field)) {
        return {};
      }
    }

    info.valid = true;
    return info;
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
    if (medium.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse) {
      sample.phiB = phiBLab;
      sample.rhoRaw = parameters.rho0 * std::pow(sample.rTilde, parameters.flowPower) * computeAffineKappaModulation(phiBLab, medium, parameters);
    } else {
      // The public knob is kappa2 = a2 / eps2_initial. Keep the legacy
      // comparison path tied to the initial eccentricity vector.
      const double a2 = parameters.kappa2 * medium.participantGeometry.eps2;
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

    double normalX = 0.0;
    double normalY = 0.0;
    if (!sampleDensityNormalDirection(medium, x, y, metric, normalX, normalY)) {
      return sample;
    }

    sample.rTilde = metric.rTilde;
    const double phiBLab = std::atan2(normalY, normalX);
    sample.phiB = phiBLab;
    if (medium.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse) {
      // In the default affine density-normal mode, anisotropy is carried by
      // the density-gradient normal itself; the explicit kappa2 multiplier is
      // now an opt-in compensation knob for matching historical strength.
      sample.rhoRaw = parameters.rho0 * std::pow(sample.rTilde, parameters.flowPower);
      if (parameters.densityNormalKappaCompensation) {
        sample.rhoRaw *= computeAffineKappaModulation(phiBLab, medium, parameters);
      }
    } else {
      sample.rhoRaw = parameters.rho0 * std::pow(sample.rTilde, parameters.flowPower);
    }
    sample.betaT = computeBetaT(sample.rhoRaw);
    sample.betaX = sample.betaT * normalX;
    sample.betaY = sample.betaT * normalY;
    return sample;
  }

  // Additive-rho mode keeps density-normal direction while applying affine closure correction to rho.
  blastwave::FlowFieldSample evaluateAffineEffectiveAdditiveRho(const blastwave::EventMedium &medium,
                                                                double x,
                                                                double y,
                                                                const blastwave::FlowFieldParameters &parameters,
                                                                const EllipseMetricSample &metric,
                                                                double hInEff,
                                                                double hOutEff,
                                                                double hBar,
                                                                double rBar) {
    blastwave::FlowFieldSample sample;
    sample.rTilde = metric.rTilde;

    double normalX = 0.0;
    double normalY = 0.0;
    if (!sampleDensityNormalDirection(medium, x, y, metric, normalX, normalY)) {
      return {};
    }

    const double radialProfile = std::pow(sample.rTilde, parameters.flowPower);
    if (!std::isfinite(radialProfile)) {
      return {};
    }

    const double xPrime = metric.deltaX * medium.emissionGeometry.minorAxisX + metric.deltaY * medium.emissionGeometry.minorAxisY;
    const double yPrime = metric.deltaX * medium.emissionGeometry.majorAxisX + metric.deltaY * medium.emissionGeometry.majorAxisY;

    const double uGeom = parameters.affineKappaFlow * radialProfile * std::sqrt((hInEff * xPrime) * (hInEff * xPrime) + (hOutEff * yPrime) * (hOutEff * yPrime));
    const double uGeomIso = parameters.affineKappaFlow * radialProfile * hBar * metric.rTilde * rBar;
    const double uGeomClipped = std::min(uGeom, parameters.affineUMax);
    const double uGeomIsoClipped = std::min(uGeomIso, parameters.affineUMax);

    const double rhoBase = parameters.rho0 * radialProfile;
    const double rhoGeom = safeAtanh(uGeomClipped);
    const double rhoGeomIso = safeAtanh(uGeomIsoClipped);
    sample.rhoRaw = std::max(0.0, rhoBase + (rhoGeom - rhoGeomIso));

    sample.betaT = std::min(parameters.affineUMax, std::tanh(sample.rhoRaw));
    sample.betaX = sample.betaT * normalX;
    sample.betaY = sample.betaT * normalY;
    sample.phiB = std::atan2(normalY, normalX);
    return sample;
  }

  // Full-tensor mode directly maps principal-axis closure into local transverse velocity.
  blastwave::FlowFieldSample evaluateAffineEffectiveFullTensor(const blastwave::EventMedium &medium,
                                                               const blastwave::FlowFieldParameters &parameters,
                                                               const EllipseMetricSample &metric,
                                                               double hInEff,
                                                               double hOutEff) {
    blastwave::FlowFieldSample sample;
    sample.rTilde = metric.rTilde;

    const double radialProfile = std::pow(sample.rTilde, parameters.flowPower);
    if (!std::isfinite(radialProfile)) {
      return {};
    }

    const double xPrime = metric.deltaX * medium.emissionGeometry.minorAxisX + metric.deltaY * medium.emissionGeometry.minorAxisY;
    const double yPrime = metric.deltaX * medium.emissionGeometry.majorAxisX + metric.deltaY * medium.emissionGeometry.majorAxisY;

    const double uXPrime = parameters.affineKappaFlow * radialProfile * hInEff * xPrime;
    const double uYPrime = parameters.affineKappaFlow * radialProfile * hOutEff * yPrime;

    double betaX = uXPrime * medium.emissionGeometry.minorAxisX + uYPrime * medium.emissionGeometry.majorAxisX;
    double betaY = uXPrime * medium.emissionGeometry.minorAxisY + uYPrime * medium.emissionGeometry.majorAxisY;
    double betaT = std::hypot(betaX, betaY);
    if (!std::isfinite(betaT)) {
      return {};
    }

    if (betaT > parameters.affineUMax) {
      const double scale = parameters.affineUMax / betaT;
      betaX *= scale;
      betaY *= scale;
      betaT = parameters.affineUMax;
    }

    sample.betaX = betaX;
    sample.betaY = betaY;
    sample.betaT = betaT;
    sample.phiB = betaT > 0.0 ? std::atan2(sample.betaY, sample.betaX) : 0.0;
    sample.rhoRaw = betaT > 0.0 ? safeAtanh(betaT) : 0.0;
    return sample;
  }

  // Route affine-effective sampling through additive-rho or full-tensor closure modes.
  blastwave::FlowFieldSample evaluateAffineEffectiveFlow(const blastwave::EventMedium &medium, double x, double y, const blastwave::FlowFieldParameters &parameters) {
    const EllipseMetricSample metric = sampleEllipseMetric(medium.emissionGeometry, x, y);
    if (!metric.valid) {
      return {};
    }

    double hInEff = 0.0;
    double hOutEff = 0.0;
    double hBar = 0.0;
    double rBar = 0.0;
    if (!computeAffineEffectiveSharedTerms(medium, parameters, hInEff, hOutEff, hBar, rBar)) {
      return {};
    }

    if (parameters.affineEffectiveMode == blastwave::AffineEffectiveMode::AdditiveRho) {
      return evaluateAffineEffectiveAdditiveRho(medium, x, y, parameters, metric, hInEff, hOutEff, hBar, rBar);
    }
    return evaluateAffineEffectiveFullTensor(medium, parameters, metric, hInEff, hOutEff);
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

  AffineEffectiveFlowInfo computeAffineEffectiveFlowInfo(const EventMedium &medium, const FlowFieldParameters &parameters) {
    return computeAffineEffectiveFlowInfoImpl(medium, parameters);
  }

  FlowFieldSample evaluateFlowField(const EventMedium &medium, double x, double y, const FlowFieldParameters &parameters) {
    switch (parameters.velocitySamplerMode) {
      case FlowVelocitySamplerMode::CovarianceEllipse:
        return evaluateCovarianceEllipseFlow(medium, x, y, parameters);
      case FlowVelocitySamplerMode::DensityNormal:
        return evaluateDensityNormalFlow(medium, x, y, parameters);
      case FlowVelocitySamplerMode::GradientResponse:
        return {};
      case FlowVelocitySamplerMode::AffineEffective:
        return evaluateAffineEffectiveFlow(medium, x, y, parameters);
    }

    return {};
  }

  FlowFieldSample evaluateFlowField(const EventMedium &medium, const EmissionSite &site, const FlowFieldParameters &parameters) {
    switch (parameters.velocitySamplerMode) {
      case FlowVelocitySamplerMode::CovarianceEllipse:
      case FlowVelocitySamplerMode::DensityNormal:
      case FlowVelocitySamplerMode::AffineEffective:
        return evaluateFlowField(medium, site.position.x, site.position.y, parameters);
      case FlowVelocitySamplerMode::GradientResponse:
        return evaluateGradientResponseFlow(site);
    }

    return {};
  }

}  // namespace blastwave
