#include "blastwave/FlowFieldModel.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "blastwave/EmissionSampler.h"
#include "blastwave/EventMedium.h"

namespace {

  constexpr double kBetaTMax = 0.95;
  constexpr double kDirectionTolerance = 1.0e-12;
  constexpr double kFlatRegionTolerance = 1.0e-6;
  constexpr double kDensityEpsilon = 1.0e-18;
  constexpr int kFlowTransPreciseAngularSamples = 360;
  constexpr int kFlowTransPreciseRadialSamples = 512;
  constexpr double kTwoPi = 6.28318530717958647692;

  struct FlowTransResolutionConfig {
    int angularSamples = kFlowTransPreciseAngularSamples;
    int radialSamples = kFlowTransPreciseRadialSamples;
  };

  // Resolve only the numerical boundary-profile grid; the density-defined
  // radius semantics stay unchanged, and Precise preserves the old 360x512 grid.
  FlowTransResolutionConfig resolveFlowTransRadiusResolution(blastwave::FlowTransRadiusResolution resolution) {
    switch (resolution) {
      case blastwave::FlowTransRadiusResolution::Balanced:
        return {240, 256};
      case blastwave::FlowTransRadiusResolution::Precise:
        return {360, 512};
      case blastwave::FlowTransRadiusResolution::Fast:
        return {120, 128};
    }
    return {kFlowTransPreciseAngularSamples, kFlowTransPreciseRadialSamples};
  }

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

  bool normalizeDirection(double &x, double &y) {
    const double magnitude = std::hypot(x, y);
    if (!std::isfinite(magnitude) || magnitude <= kDirectionTolerance) {
      return false;
    }
    x /= magnitude;
    y /= magnitude;
    return true;
  }

  // Recover the geometric radial direction from the emission-geometry center.
  bool sampleGeometricRadialDirection(const blastwave::FlowEllipseInfo &ellipse, double x, double y, double &directionX, double &directionY) {
    directionX = x - ellipse.centerX;
    directionY = y - ellipse.centerY;
    return normalizeDirection(directionX, directionY);
  }

  // Recover the outward density-gradient direction without covariance fallback.
  bool sampleDensityGradientDirection(const blastwave::EventMedium &medium, double x, double y, double &directionX, double &directionY) {
    const blastwave::DensityFieldSample densitySample = blastwave::evaluateDensityField(medium.emissionDensity, x, y);
    const double gradientMagnitude = std::hypot(densitySample.gradientX, densitySample.gradientY);
    const double flatness = gradientMagnitude * medium.emissionDensity.gaussianSigma / std::max(densitySample.density, kDensityEpsilon);
    if (!std::isfinite(gradientMagnitude) || gradientMagnitude <= kDirectionTolerance || !std::isfinite(flatness) || flatness < kFlatRegionTolerance) {
      return false;
    }

    directionX = -densitySample.gradientX / gradientMagnitude;
    directionY = -densitySample.gradientY / gradientMagnitude;
    return normalizeDirection(directionX, directionY);
  }

  bool hasDensityNormalFlowTransOverrides(const blastwave::FlowFieldParameters &parameters) {
    return parameters.hasFlowTransDirectionGradientFraction || parameters.hasFlowTransRadius;
  }

  // Resolve the configured fallback chain for mixed density-normal direction.
  bool sampleConfiguredDensityNormalDirection(const blastwave::EventMedium &medium,
                                              double x,
                                              double y,
                                              const blastwave::FlowFieldParameters &parameters,
                                              const EllipseMetricSample &metric,
                                              double &normalX,
                                              double &normalY) {
    double geometricX = 0.0;
    double geometricY = 0.0;
    const bool hasGeometricDirection = sampleGeometricRadialDirection(medium.emissionGeometry, x, y, geometricX, geometricY);

    double gradientX = 0.0;
    double gradientY = 0.0;
    const bool hasGradientDirection = sampleDensityGradientDirection(medium, x, y, gradientX, gradientY);

    const double gradientFraction = parameters.flowTransDirectionGradientFraction;
    if (hasGeometricDirection && hasGradientDirection) {
      const double geometricWeight = 1.0 - gradientFraction;
      double mixedX = geometricWeight * geometricX + gradientFraction * gradientX;
      double mixedY = geometricWeight * geometricY + gradientFraction * gradientY;
      if (normalizeDirection(mixedX, mixedY)) {
        normalX = mixedX;
        normalY = mixedY;
        return true;
      }
    }

    if (hasGradientDirection) {
      normalX = gradientX;
      normalY = gradientY;
      return true;
    }
    if (hasGeometricDirection) {
      normalX = geometricX;
      normalY = geometricY;
      return true;
    }
    if (metric.valid) {
      normalX = metric.normalX;
      normalY = metric.normalY;
      return normalizeDirection(normalX, normalY);
    }
    return false;
  }

  // Estimate a finite ray marching range from event geometry and density support.
  double computeDensityRayUpperBound(const blastwave::EventMedium &medium) {
    if (!medium.emissionGeometry.valid) {
      return 0.0;
    }

    const double centerX = medium.emissionGeometry.centerX;
    const double centerY = medium.emissionGeometry.centerY;
    double supportRadiusMax = 0.0;
    for (const blastwave::WeightedTransversePoint &point : medium.emissionDensity.supportPoints) {
      if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
        continue;
      }
      supportRadiusMax = std::max(supportRadiusMax, std::hypot(point.x - centerX, point.y - centerY));
    }

    const double sigmaTail = std::max(0.0, medium.emissionDensity.gaussianSigma);
    double upperBound = supportRadiusMax + 8.0 * sigmaTail;
    if (!std::isfinite(upperBound) || upperBound <= kDirectionTolerance) {
      upperBound = std::max(1.0, 4.0 * std::max(medium.emissionGeometry.radiusMajor, medium.emissionGeometry.radiusMinor));
    }
    return upperBound;
  }

  // Interpolate one radial threshold crossing on a density ray segment.
  double interpolateThresholdCrossing(double r0, double d0, double r1, double d1, double threshold) {
    if (!std::isfinite(r0) || !std::isfinite(r1) || !std::isfinite(d0) || !std::isfinite(d1)) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    const double deltaDensity = d1 - d0;
    if (std::abs(deltaDensity) <= kDensityEpsilon) {
      return r0;
    }
    const double weight = (threshold - d0) / deltaDensity;
    if (!std::isfinite(weight)) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return std::clamp(r0 + (r1 - r0) * weight, std::min(r0, r1), std::max(r0, r1));
  }

  // Find the percentile boundary radius from ray-integrated polar density.
  double computeDensityPercentileBoundaryRadius(const blastwave::EventMedium &medium,
                                               double directionX,
                                               double directionY,
                                               double percentileFraction,
                                               double radiusUpperBound,
                                               int radialSamples,
                                               std::vector<double> &cumulativeIntegrals) {
    if (!(radiusUpperBound > 0.0) || !std::isfinite(radiusUpperBound)) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    if (radialSamples <= 0) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    const double radialStep = radiusUpperBound / static_cast<double>(radialSamples);
    if (!(radialStep > 0.0) || !std::isfinite(radialStep)) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const double centerX = medium.emissionGeometry.centerX;
    const double centerY = medium.emissionGeometry.centerY;
    if (static_cast<int>(cumulativeIntegrals.size()) != radialSamples + 1) {
      cumulativeIntegrals.assign(static_cast<std::size_t>(radialSamples + 1), 0.0);
    } else {
      std::fill(cumulativeIntegrals.begin(), cumulativeIntegrals.end(), 0.0);
    }
    double previousWeight = 0.0;

    for (int iSample = 1; iSample <= radialSamples; ++iSample) {
      const double radius = radialStep * static_cast<double>(iSample);
      const double density = blastwave::evaluateDensityValue(medium.emissionDensity, centerX + radius * directionX, centerY + radius * directionY);
      const double weight = radius * std::max(0.0, density);
      cumulativeIntegrals[static_cast<std::size_t>(iSample)] =
          cumulativeIntegrals[static_cast<std::size_t>(iSample - 1)] + 0.5 * (previousWeight + weight) * radialStep;
      previousWeight = weight;
    }

    const double totalIntegral = cumulativeIntegrals.back();
    if (!std::isfinite(totalIntegral) || totalIntegral <= kDensityEpsilon) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const double targetIntegral = percentileFraction * totalIntegral;
    for (int iSample = 1; iSample <= radialSamples; ++iSample) {
      const std::size_t iCurrent = static_cast<std::size_t>(iSample);
      if (cumulativeIntegrals[iCurrent] < targetIntegral) {
        continue;
      }
      const double leftIntegral = cumulativeIntegrals[iCurrent - 1U];
      const double rightIntegral = cumulativeIntegrals[iCurrent];
      const double leftRadius = radialStep * static_cast<double>(iCurrent - 1U);
      const double rightRadius = radialStep * static_cast<double>(iCurrent);
      if (std::abs(rightIntegral - leftIntegral) <= kDensityEpsilon) {
        return rightRadius;
      }
      const double interpolation = (targetIntegral - leftIntegral) / (rightIntegral - leftIntegral);
      return leftRadius + (rightRadius - leftRadius) * std::clamp(interpolation, 0.0, 1.0);
    }

    return radiusUpperBound;
  }

  // Find the outermost density-level boundary radius on one angular ray.
  double computeDensityLevelBoundaryRadius(const blastwave::EventMedium &medium,
                                           double directionX,
                                           double directionY,
                                           double levelFraction,
                                           double radiusUpperBound,
                                           int radialSamples) {
    if (!(radiusUpperBound > 0.0) || !std::isfinite(radiusUpperBound)) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    if (radialSamples <= 0) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const double densityThreshold = levelFraction * medium.emissionDensityScale;
    if (!std::isfinite(densityThreshold) || densityThreshold <= 0.0) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const double radialStep = radiusUpperBound / static_cast<double>(radialSamples);
    if (!(radialStep > 0.0) || !std::isfinite(radialStep)) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const double centerX = medium.emissionGeometry.centerX;
    const double centerY = medium.emissionGeometry.centerY;
    double previousRadius = 0.0;
    double previousDensity = blastwave::evaluateDensityValue(medium.emissionDensity, centerX, centerY);
    double boundaryRadius = std::numeric_limits<double>::quiet_NaN();

    for (int iSample = 1; iSample <= radialSamples; ++iSample) {
      const double radius = radialStep * static_cast<double>(iSample);
      const double density = blastwave::evaluateDensityValue(medium.emissionDensity, centerX + radius * directionX, centerY + radius * directionY);
      const bool previousInside = std::isfinite(previousDensity) && previousDensity >= densityThreshold;
      const bool currentInside = std::isfinite(density) && density >= densityThreshold;
      if (previousInside && currentInside) {
        boundaryRadius = radius;
      } else if (previousInside && !currentInside) {
        const double crossingRadius = interpolateThresholdCrossing(previousRadius, previousDensity, radius, density, densityThreshold);
        if (std::isfinite(crossingRadius)) {
          boundaryRadius = crossingRadius;
        }
      } else if (!previousInside && currentInside) {
        boundaryRadius = radius;
      }
      previousRadius = radius;
      previousDensity = density;
    }

    return boundaryRadius;
  }

  // Build or reuse the per-event angular boundary profile for density-defined radii.
  const blastwave::FlowTransRadiusProfile &getFlowTransRadiusProfile(
      const blastwave::EventMedium &medium, const blastwave::FlowFieldParameters &parameters) {
    const auto resolution = resolveFlowTransRadiusResolution(parameters.flowTransRadiusResolution);
    blastwave::FlowTransRadiusProfile &profile = medium.flowTransRadiusProfile;
    const int angularSamples = resolution.angularSamples;
    const int radialSamples = resolution.radialSamples;
    const bool cacheMatches = profile.valid && profile.mode == parameters.flowTransRadiusMode
                              && profile.fraction == parameters.flowTransRadiusFraction
                              && profile.centerX == medium.emissionGeometry.centerX
                              && profile.centerY == medium.emissionGeometry.centerY
                              && profile.resolution == parameters.flowTransRadiusResolution
                              && profile.angularSamples == angularSamples
                              && profile.radialSamples == radialSamples
                              && profile.boundaryRadii.size() == static_cast<std::size_t>(angularSamples);
    if (cacheMatches) {
      return profile;
    }

    const double radiusUpperBound = computeDensityRayUpperBound(medium);
    profile = {};
    profile.mode = parameters.flowTransRadiusMode;
    profile.fraction = parameters.flowTransRadiusFraction;
    profile.centerX = medium.emissionGeometry.centerX;
    profile.centerY = medium.emissionGeometry.centerY;
    profile.radiusUpperBound = radiusUpperBound;
    profile.resolution = parameters.flowTransRadiusResolution;
    profile.angularSamples = angularSamples;
    profile.radialSamples = radialSamples;
    profile.boundaryRadii.assign(static_cast<std::size_t>(angularSamples), std::numeric_limits<double>::quiet_NaN());
    if (angularSamples <= 0 || radialSamples <= 0) {
      return profile;
    }

    if (!(radiusUpperBound > 0.0) || !std::isfinite(radiusUpperBound)) {
      return profile;
    }

    bool hasFiniteBoundary = false;
    std::vector<double> cumulativeIntegrals(static_cast<std::size_t>(radialSamples + 1), 0.0);
    for (int iAngle = 0; iAngle < angularSamples; ++iAngle) {
      const double angle = kTwoPi * static_cast<double>(iAngle) / static_cast<double>(angularSamples);
      const double directionX = std::cos(angle);
      const double directionY = std::sin(angle);
      double boundaryRadius = std::numeric_limits<double>::quiet_NaN();
      if (parameters.flowTransRadiusMode == blastwave::FlowTransRadiusMode::DensityPercentile) {
        boundaryRadius = computeDensityPercentileBoundaryRadius(
            medium, directionX, directionY, parameters.flowTransRadiusFraction, radiusUpperBound, radialSamples, cumulativeIntegrals);
      } else if (parameters.flowTransRadiusMode == blastwave::FlowTransRadiusMode::DensityLevel) {
        boundaryRadius = computeDensityLevelBoundaryRadius(medium, directionX, directionY, parameters.flowTransRadiusFraction, radiusUpperBound, radialSamples);
      }

      if (std::isfinite(boundaryRadius) && boundaryRadius > kDirectionTolerance) {
        profile.boundaryRadii[static_cast<std::size_t>(iAngle)] = boundaryRadius;
        hasFiniteBoundary = true;
      }
    }

    profile.valid = hasFiniteBoundary;
    return profile;
  }

  // Interpolate the cached angular profile at the point's geometric ray direction.
  double sampleBoundaryRadiusFromProfile(const blastwave::FlowTransRadiusProfile &profile, double directionX, double directionY) {
    if (!profile.valid || profile.angularSamples <= 0 || profile.boundaryRadii.empty()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const double directionNorm = std::hypot(directionX, directionY);
    if (!std::isfinite(directionNorm) || directionNorm <= kDirectionTolerance) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    double angle = std::atan2(directionY / directionNorm, directionX / directionNorm);
    if (angle < 0.0) {
      angle += kTwoPi;
    }
    const double scaledIndex = angle * static_cast<double>(profile.angularSamples) / kTwoPi;
    const int leftIndex = static_cast<int>(std::floor(scaledIndex)) % profile.angularSamples;
    const int rightIndex = (leftIndex + 1) % profile.angularSamples;
    const double interpolation = scaledIndex - std::floor(scaledIndex);

    const double leftRadius = profile.boundaryRadii[static_cast<std::size_t>(leftIndex)];
    const double rightRadius = profile.boundaryRadii[static_cast<std::size_t>(rightIndex)];
    if (std::isfinite(leftRadius) && std::isfinite(rightRadius)) {
      return leftRadius + (rightRadius - leftRadius) * std::clamp(interpolation, 0.0, 1.0);
    }
    if (std::isfinite(leftRadius)) {
      return leftRadius;
    }
    if (std::isfinite(rightRadius)) {
      return rightRadius;
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Convert one point into xi_used for density-defined transverse radii.
  double computeDensityDefinedXiUsed(
      const blastwave::EventMedium &medium, double x, double y, const blastwave::FlowFieldParameters &parameters) {
    double rayX = 0.0;
    double rayY = 0.0;
    if (!sampleGeometricRadialDirection(medium.emissionGeometry, x, y, rayX, rayY)) {
      return 0.0;
    }

    if (parameters.flowTransRadiusMode != blastwave::FlowTransRadiusMode::DensityPercentile
        && parameters.flowTransRadiusMode != blastwave::FlowTransRadiusMode::DensityLevel) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const blastwave::FlowTransRadiusProfile &profile = getFlowTransRadiusProfile(medium, parameters);
    const double boundaryRadius = sampleBoundaryRadiusFromProfile(profile, rayX, rayY);
    if (!std::isfinite(boundaryRadius) || boundaryRadius <= kDirectionTolerance) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    const double radialDistance = std::hypot(x - medium.emissionGeometry.centerX, y - medium.emissionGeometry.centerY);
    if (!std::isfinite(radialDistance) || radialDistance <= 0.0) {
      return 0.0;
    }

    const double xi = radialDistance / boundaryRadius;
    return std::min(1.0, std::max(0.0, xi));
  }

  // Build shared affine-effective closure values used by both runtime modes.
  bool computeAffineEffectiveSharedTerms(
      const blastwave::EventMedium &medium, const blastwave::FlowFieldParameters &parameters, double &hInEff, double &hOutEff, double &hBar, double &rBar) {
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
      // Additive-rho mode keeps a transverse rapidity baseline plus affine geometry correction.
      const double uGeomIso = parameters.affineKappaFlow * hBar * rBar;
      const double uGeomIn = parameters.affineKappaFlow * std::sqrt((hInEff * medium.affineEffectiveClosure.sigmaInFinal) * (hInEff * medium.affineEffectiveClosure.sigmaInFinal));
      const double uGeomOut =
          parameters.affineKappaFlow * std::sqrt((hOutEff * medium.affineEffectiveClosure.sigmaOutFinal) * (hOutEff * medium.affineEffectiveClosure.sigmaOutFinal));
      const double uGeomIsoClipped = std::min(uGeomIso, parameters.affineUMax);
      const double uGeomInClipped = std::min(uGeomIn, parameters.affineUMax);
      const double uGeomOutClipped = std::min(uGeomOut, parameters.affineUMax);

      info.surfaceRhoBase = parameters.flowTransRho0;
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
      sample.rhoRaw =
          parameters.flowTransRho0 * std::pow(sample.rTilde, parameters.flowTransProfilePower) * computeAffineKappaModulation(phiBLab, medium, parameters);
    } else {
      // The public knob is kappa2 = a2 / eps2_initial. Keep the legacy
      // comparison path tied to the initial eccentricity vector.
      const double a2 = parameters.kappa2 * medium.participantGeometry.eps2;
      sample.phiB = std::atan2(metric.qMinor, metric.qMajor);
      sample.rhoRaw = std::pow(sample.rTilde, parameters.flowTransProfilePower) * (parameters.flowTransRho0 + a2 * std::cos(2.0 * sample.phiB));
    }
    sample.betaT = computeBetaT(sample.rhoRaw);
    sample.betaX = sample.betaT * metric.normalX;
    sample.betaY = sample.betaT * metric.normalY;
    return sample;
  }

  // Sample density-normal flow with legacy behavior by default and a
  // configured mixed-direction plus density-defined radius path when enabled.
  blastwave::FlowFieldSample evaluateDensityNormalFlow(const blastwave::EventMedium &medium, double x, double y, const blastwave::FlowFieldParameters &parameters) {
    blastwave::FlowFieldSample sample;
    const EllipseMetricSample metric = sampleEllipseMetric(medium.emissionGeometry, x, y);
    if (!metric.valid) {
      return sample;
    }

    double normalX = 0.0;
    double normalY = 0.0;
    sample.rTilde = metric.rTilde;

    // Preserve historical density-normal behavior unless an explicit
    // direction/radius flow-trans override has been provided.
    if (!hasDensityNormalFlowTransOverrides(parameters)) {
      if (!sampleDensityNormalDirection(medium, x, y, metric, normalX, normalY)) {
        return sample;
      }
      const double phiBLab = std::atan2(normalY, normalX);
      sample.phiB = phiBLab;
      sample.rhoRaw = parameters.flowTransRho0 * std::pow(sample.rTilde, parameters.flowTransProfilePower);
      if (medium.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse && parameters.densityNormalKappaCompensation) {
        sample.rhoRaw *= computeAffineKappaModulation(phiBLab, medium, parameters);
      }
      sample.betaT = computeBetaT(sample.rhoRaw);
      sample.betaX = sample.betaT * normalX;
      sample.betaY = sample.betaT * normalY;
      return sample;
    }

    if (!sampleConfiguredDensityNormalDirection(medium, x, y, parameters, metric, normalX, normalY)) {
      return sample;
    }

    double xiUsed = metric.rTilde;
    if (parameters.flowTransRadiusMode == blastwave::FlowTransRadiusMode::DensityPercentile
        || parameters.flowTransRadiusMode == blastwave::FlowTransRadiusMode::DensityLevel) {
      xiUsed = computeDensityDefinedXiUsed(medium, x, y, parameters);
      if (!std::isfinite(xiUsed)) {
        xiUsed = metric.rTilde;
      }
    }

    if (!std::isfinite(xiUsed) || xiUsed < 0.0) {
      return {};
    }
    const double radialProfile = std::pow(xiUsed, parameters.flowTransProfilePower);
    if (!std::isfinite(radialProfile)) {
      return {};
    }

    sample.phiB = std::atan2(normalY, normalX);
    sample.rhoRaw = parameters.flowTransRho0 * radialProfile;
    if (medium.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse && parameters.densityNormalKappaCompensation) {
      sample.rhoRaw *= computeAffineKappaModulation(sample.phiB, medium, parameters);
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

    const double radialProfile = std::pow(sample.rTilde, parameters.flowTransProfilePower);
    if (!std::isfinite(radialProfile)) {
      return {};
    }

    const double xPrime = metric.deltaX * medium.emissionGeometry.minorAxisX + metric.deltaY * medium.emissionGeometry.minorAxisY;
    const double yPrime = metric.deltaX * medium.emissionGeometry.majorAxisX + metric.deltaY * medium.emissionGeometry.majorAxisY;

    const double uGeom = parameters.affineKappaFlow * radialProfile * std::sqrt((hInEff * xPrime) * (hInEff * xPrime) + (hOutEff * yPrime) * (hOutEff * yPrime));
    const double uGeomIso = parameters.affineKappaFlow * radialProfile * hBar * metric.rTilde * rBar;
    const double uGeomClipped = std::min(uGeom, parameters.affineUMax);
    const double uGeomIsoClipped = std::min(uGeomIso, parameters.affineUMax);

    const double rhoBase = parameters.flowTransRho0 * radialProfile;
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
  blastwave::FlowFieldSample evaluateAffineEffectiveFullTensor(
      const blastwave::EventMedium &medium, const blastwave::FlowFieldParameters &parameters, const EllipseMetricSample &metric, double hInEff, double hOutEff) {
    blastwave::FlowFieldSample sample;
    sample.rTilde = metric.rTilde;

    const double radialProfile = std::pow(sample.rTilde, parameters.flowTransProfilePower);
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
