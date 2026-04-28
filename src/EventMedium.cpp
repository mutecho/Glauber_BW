#include "blastwave/EventMedium.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace {

  constexpr double kEllipseTolerance = 1.0e-12;

  bool lexicographicallyNegative(double x, double y) {
    return x < 0.0 || (std::abs(x) <= kEllipseTolerance && y < 0.0);
  }

  int countContributingPoints(const std::vector<blastwave::WeightedTransversePoint> &points) {
    int count = 0;
    for (const blastwave::WeightedTransversePoint &point : points) {
      if (std::isfinite(point.weight) && point.weight > 0.0) {
        ++count;
      }
    }
    return count;
  }

  // Recover a stable density normalization from finite positive support-point
  // values so gradient samplers can use robust event-level floors/cutoffs.
  double computeSupportDensityScale(const blastwave::DensityField &field) {
    double scale = 0.0;
    for (const blastwave::WeightedTransversePoint &point : field.supportPoints) {
      const blastwave::DensityFieldSample sample = blastwave::evaluateDensityField(field, point.x, point.y);
      if (std::isfinite(sample.density) && sample.density > scale) {
        scale = sample.density;
      }
    }
    return scale;
  }

  // Recompute all cached ellipse invariants after covariance updates so
  // freeze-out geometry and flow samplers stay analytically consistent.
  void finalizeEllipseFromCovariance(blastwave::FlowEllipseInfo &ellipse, int contributingPoints) {
    const double trace = ellipse.sigmaX2 + ellipse.sigmaY2;
    if (trace > kEllipseTolerance) {
      const double eccX = ellipse.sigmaY2 - ellipse.sigmaX2;
      const double eccY = 2.0 * ellipse.sigmaXY;
      ellipse.eps2 = std::hypot(eccX, eccY) / trace;
      ellipse.psi2 = 0.5 * std::atan2(eccY, eccX);
    } else {
      ellipse.eps2 = 0.0;
      ellipse.psi2 = 0.0;
    }

    const double covarianceDiscriminant =
        std::sqrt(std::max(0.0, (ellipse.sigmaX2 - ellipse.sigmaY2) * (ellipse.sigmaX2 - ellipse.sigmaY2) + 4.0 * ellipse.sigmaXY * ellipse.sigmaXY));
    ellipse.lambdaMajor = 0.5 * (trace + covarianceDiscriminant);
    ellipse.lambdaMinor = 0.5 * (trace - covarianceDiscriminant);
    ellipse.radiusMajor = ellipse.lambdaMajor > 0.0 ? std::sqrt(ellipse.lambdaMajor) : 0.0;
    ellipse.radiusMinor = ellipse.lambdaMinor > 0.0 ? std::sqrt(ellipse.lambdaMinor) : 0.0;

    const double determinant = ellipse.sigmaX2 * ellipse.sigmaY2 - ellipse.sigmaXY * ellipse.sigmaXY;
    if (determinant > kEllipseTolerance) {
      ellipse.inverseSigmaXX = ellipse.sigmaY2 / determinant;
      ellipse.inverseSigmaXY = -ellipse.sigmaXY / determinant;
      ellipse.inverseSigmaYY = ellipse.sigmaX2 / determinant;
    } else {
      ellipse.inverseSigmaXX = 0.0;
      ellipse.inverseSigmaXY = 0.0;
      ellipse.inverseSigmaYY = 0.0;
    }

    const double majorAxisAngle = 0.5 * std::atan2(2.0 * ellipse.sigmaXY, ellipse.sigmaX2 - ellipse.sigmaY2);
    ellipse.majorAxisX = std::cos(majorAxisAngle);
    ellipse.majorAxisY = std::sin(majorAxisAngle);

    if (ellipse.eps2 > kEllipseTolerance) {
      const double preferredMinorX = std::cos(ellipse.psi2);
      const double preferredMinorY = std::sin(ellipse.psi2);
      const double currentMinorX = -ellipse.majorAxisY;
      const double currentMinorY = ellipse.majorAxisX;
      if (currentMinorX * preferredMinorX + currentMinorY * preferredMinorY < 0.0) {
        ellipse.majorAxisX = -ellipse.majorAxisX;
        ellipse.majorAxisY = -ellipse.majorAxisY;
      }
    } else if (lexicographicallyNegative(ellipse.majorAxisX, ellipse.majorAxisY)) {
      ellipse.majorAxisX = -ellipse.majorAxisX;
      ellipse.majorAxisY = -ellipse.majorAxisY;
    }

    ellipse.minorAxisX = -ellipse.majorAxisY;
    ellipse.minorAxisY = ellipse.majorAxisX;
    ellipse.valid = contributingPoints >= 2 && trace > kEllipseTolerance && ellipse.lambdaMajor > kEllipseTolerance && ellipse.lambdaMinor > kEllipseTolerance
                    && determinant > kEllipseTolerance;
  }

  // Apply the freeze-out affine response in the participant-plane basis.
  std::vector<blastwave::WeightedTransversePoint> applyAffineResponse(const std::vector<blastwave::WeightedTransversePoint> &points,
                                                                      const blastwave::FlowEllipseInfo &participantGeometry,
                                                                      double lambdaIn,
                                                                      double lambdaOut) {
    std::vector<blastwave::WeightedTransversePoint> transformed;
    transformed.reserve(points.size());

    for (const blastwave::WeightedTransversePoint &point : points) {
      const double dx = point.x - participantGeometry.centerX;
      const double dy = point.y - participantGeometry.centerY;
      const double inPlane = dx * participantGeometry.minorAxisX + dy * participantGeometry.minorAxisY;
      const double outOfPlane = dx * participantGeometry.majorAxisX + dy * participantGeometry.majorAxisY;
      const double transformedDx = lambdaIn * inPlane * participantGeometry.minorAxisX + lambdaOut * outOfPlane * participantGeometry.majorAxisX;
      const double transformedDy = lambdaIn * inPlane * participantGeometry.minorAxisY + lambdaOut * outOfPlane * participantGeometry.majorAxisY;
      transformed.push_back({participantGeometry.centerX + transformedDx, participantGeometry.centerY + transformedDy, point.weight});
    }

    return transformed;
  }

  // Build the shared full kernel covariance A*sigma_dep^2*I*A^T + sigma_evo^2*I.
  std::array<double, 3> computeAffineKernelCovariance(const blastwave::FlowEllipseInfo &participantGeometry,
                                                       double densitySigma,
                                                       double lambdaIn,
                                                       double lambdaOut,
                                                       double sigmaEvo) {
    const double sigmaDep2 = densitySigma * densitySigma;
    const double sigmaEvo2 = sigmaEvo * sigmaEvo;
    const double lambdaIn2 = lambdaIn * lambdaIn;
    const double lambdaOut2 = lambdaOut * lambdaOut;
    const double covXX =
        sigmaDep2 * (lambdaIn2 * participantGeometry.minorAxisX * participantGeometry.minorAxisX + lambdaOut2 * participantGeometry.majorAxisX * participantGeometry.majorAxisX)
        + sigmaEvo2;
    const double covYY =
        sigmaDep2 * (lambdaIn2 * participantGeometry.minorAxisY * participantGeometry.minorAxisY + lambdaOut2 * participantGeometry.majorAxisY * participantGeometry.majorAxisY)
        + sigmaEvo2;
    const double covXY =
        sigmaDep2 * (lambdaIn2 * participantGeometry.minorAxisX * participantGeometry.minorAxisY + lambdaOut2 * participantGeometry.majorAxisX * participantGeometry.majorAxisY);
    return {covXX, covXY, covYY};
  }

}  // namespace

namespace blastwave {

  // Construct the event medium in two named stages while preserving the
  // participant-geometry summary contract used by event-level diagnostics.
  EventMedium buildEventMedium(const std::vector<WeightedTransversePoint> &points, const EventMediumParameters &parameters) {
    EventMedium medium;
    medium.densityEvolutionMode = parameters.densityEvolutionMode;
    medium.participantPoints = points;
    medium.participantGeometry = computeFlowEllipseInfo(points);
    medium.initialDensity = buildGaussianPointCloudDensityField(points, parameters.densitySigma);

    switch (parameters.densityEvolutionMode) {
      case DensityEvolutionMode::None:
        medium.markerDensity = medium.initialDensity;
        medium.dynamicsDensity = medium.initialDensity;
        medium.emissionDensity = medium.initialDensity;
        medium.emissionGeometry = medium.participantGeometry;
        break;
      case DensityEvolutionMode::AffineGaussianResponse: {
        const std::vector<WeightedTransversePoint> transformedPoints =
            applyAffineResponse(points, medium.participantGeometry, parameters.affineLambdaIn, parameters.affineLambdaOut);
        const std::array<double, 3> kernelCovariance = computeAffineKernelCovariance(
            medium.participantGeometry, parameters.densitySigma, parameters.affineLambdaIn, parameters.affineLambdaOut, parameters.affineSigmaEvo);
        medium.emissionDensity =
            buildGaussianPointCloudDensityField(transformedPoints, kernelCovariance[0], kernelCovariance[1], kernelCovariance[2]);
        medium.markerDensity = medium.emissionDensity;
        medium.dynamicsDensity = medium.emissionDensity;

        // Fold the support cloud covariance and shared kernel covariance into
        // a freeze-out geometry proxy used by flow modulation and diagnostics.
        medium.emissionGeometry = computeFlowEllipseInfo(transformedPoints);
        medium.emissionGeometry.sigmaX2 += kernelCovariance[0];
        medium.emissionGeometry.sigmaXY += kernelCovariance[1];
        medium.emissionGeometry.sigmaY2 += kernelCovariance[2];
        finalizeEllipseFromCovariance(medium.emissionGeometry, countContributingPoints(transformedPoints));
        break;
      }
      case DensityEvolutionMode::GradientResponse: {
        const double sigmaDep2 = parameters.densitySigma * parameters.densitySigma;
        const double sigmaEmEff = std::sqrt(std::max(0.0, sigmaDep2 + parameters.gradientSigmaEm * parameters.gradientSigmaEm));
        const double sigmaDynEff = std::sqrt(std::max(0.0, sigmaDep2 + parameters.gradientSigmaDyn * parameters.gradientSigmaDyn));

        // Keep marker and dynamics densities as separate smoothings of s0.
        medium.markerDensity = buildGaussianPointCloudDensityField(points, sigmaEmEff);
        medium.dynamicsDensity = buildGaussianPointCloudDensityField(points, sigmaDynEff);

        // Preserve legacy consumers by exposing the marker density as s_f seed.
        medium.emissionDensity = medium.markerDensity;
        medium.emissionGeometry = medium.participantGeometry;
        break;
      }
    }

    medium.markerDensityScale = computeSupportDensityScale(medium.markerDensity);
    medium.dynamicsDensityScale = computeSupportDensityScale(medium.dynamicsDensity);

    return medium;
  }

}  // namespace blastwave
