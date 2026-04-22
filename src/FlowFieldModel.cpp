#include "blastwave/FlowFieldModel.h"

#include <algorithm>
#include <cmath>

namespace {

  constexpr double kEllipseTolerance = 1.0e-12;
  constexpr double kBetaTMax = 0.95;

  bool lexicographicallyNegative(double x, double y) {
    return x < 0.0 || (std::abs(x) <= kEllipseTolerance && y < 0.0);
  }

}  // namespace

namespace blastwave {

  // Recover the participant covariance ellipse, preserve the historical
  // eps2/psi2 summary convention, and make the principal-axis orientation
  // deterministic for tests and optional debug serialization.
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

    ellipse.valid = contributingPoints >= 2 && trace > kEllipseTolerance && ellipse.lambdaMajor > kEllipseTolerance && ellipse.lambdaMinor > kEllipseTolerance;
    return ellipse;
  }

  // Evaluate the default flow field at one transverse point, using the
  // ellipse-normal direction and clipping the transverse speed before the
  // generator composes in longitudinal Bjorken motion.
  FlowFieldSample evaluateFlowField(const FlowEllipseInfo &ellipse, double x, double y, const FlowFieldParameters &parameters) {
    FlowFieldSample sample;
    if (!ellipse.valid) {
      return sample;
    }

    const double dx = x - ellipse.centerX;
    const double dy = y - ellipse.centerY;
    const double xPrime = dx * ellipse.majorAxisX + dy * ellipse.majorAxisY;
    const double yPrime = dx * ellipse.minorAxisX + dy * ellipse.minorAxisY;

    sample.rTilde = std::sqrt(xPrime * xPrime / (ellipse.radiusMajor * ellipse.radiusMajor) + yPrime * yPrime / (ellipse.radiusMinor * ellipse.radiusMinor));
    sample.phiB = std::atan2(yPrime / (ellipse.radiusMinor * ellipse.radiusMinor), xPrime / (ellipse.radiusMajor * ellipse.radiusMajor));
    sample.rhoRaw = std::pow(sample.rTilde, parameters.flowPower) * (parameters.rho0 + parameters.rho2 * ellipse.eps2 * std::cos(2.0 * sample.phiB));
    sample.betaT = std::min(kBetaTMax, std::tanh(std::max(0.0, sample.rhoRaw)));

    const double betaMajor = sample.betaT * std::cos(sample.phiB);
    const double betaMinor = sample.betaT * std::sin(sample.phiB);
    sample.betaX = betaMajor * ellipse.majorAxisX + betaMinor * ellipse.minorAxisX;
    sample.betaY = betaMajor * ellipse.majorAxisY + betaMinor * ellipse.minorAxisY;
    return sample;
  }

}  // namespace blastwave
