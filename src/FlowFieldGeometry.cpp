#include "blastwave/FlowFieldModel.h"

#include <algorithm>
#include <cmath>

namespace {

  constexpr double kEllipseTolerance = 1.0e-12;

  bool lexicographicallyNegative(double x, double y) {
    return x < 0.0 || (std::abs(x) <= kEllipseTolerance && y < 0.0);
  }

}  // namespace

namespace blastwave {

  // Recover the participant covariance ellipse, cache the inverse covariance,
  // and keep the deterministic axis orientation needed by tests and debug I/O.
  FlowEllipseInfo computeFlowEllipseInfo(const std::vector<WeightedTransversePoint> &points) {
    FlowEllipseInfo ellipse;

    double totalWeight = 0.0;
    int contributingPoints = 0;
    for (const WeightedTransversePoint &point : points) {
      if (!std::isfinite(point.weight) || point.weight <= 0.0) {
        continue;
      }

      ellipse.centerX += point.weight * point.x;
      ellipse.centerY += point.weight * point.y;
      totalWeight += point.weight;
      ++contributingPoints;
    }

    if (totalWeight <= 0.0) {
      return ellipse;
    }

    ellipse.centerX /= totalWeight;
    ellipse.centerY /= totalWeight;

    for (const WeightedTransversePoint &point : points) {
      if (!std::isfinite(point.weight) || point.weight <= 0.0) {
        continue;
      }

      const double dx = point.x - ellipse.centerX;
      const double dy = point.y - ellipse.centerY;
      ellipse.sigmaX2 += point.weight * dx * dx;
      ellipse.sigmaY2 += point.weight * dy * dy;
      ellipse.sigmaXY += point.weight * dx * dy;
    }

    ellipse.sigmaX2 /= totalWeight;
    ellipse.sigmaY2 /= totalWeight;
    ellipse.sigmaXY /= totalWeight;

    const double trace = ellipse.sigmaX2 + ellipse.sigmaY2;
    if (trace > kEllipseTolerance) {
      const double eccX = ellipse.sigmaY2 - ellipse.sigmaX2;
      const double eccY = 2.0 * ellipse.sigmaXY;
      ellipse.eps2 = std::hypot(eccX, eccY) / trace;
      ellipse.psi2 = 0.5 * std::atan2(eccY, eccX);
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
    return ellipse;
  }

}  // namespace blastwave
