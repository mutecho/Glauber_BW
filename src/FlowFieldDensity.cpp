#include "blastwave/DensityFieldModel.h"

#include <cmath>

namespace {

  constexpr double kPi = 3.14159265358979323846;

}  // namespace

namespace blastwave {

  DensityField buildGaussianPointCloudDensityField(const std::vector<WeightedTransversePoint> &points, double gaussianSigma) {
    DensityField field;
    field.supportPoints = points;
    field.gaussianSigma = gaussianSigma;
    const double sigma2 = gaussianSigma * gaussianSigma;
    field.kernelCovXX = sigma2;
    field.kernelCovXY = 0.0;
    field.kernelCovYY = sigma2;
    return field;
  }

  DensityField buildGaussianPointCloudDensityField(const std::vector<WeightedTransversePoint> &points,
                                                   double kernelCovXX,
                                                   double kernelCovXY,
                                                   double kernelCovYY) {
    DensityField field;
    field.supportPoints = points;
    field.kernelCovXX = kernelCovXX;
    field.kernelCovXY = kernelCovXY;
    field.kernelCovYY = kernelCovYY;
    const double averageVariance = 0.5 * (kernelCovXX + kernelCovYY);
    field.gaussianSigma = averageVariance > 0.0 ? std::sqrt(averageVariance) : 0.0;
    return field;
  }

  // Evaluate the analytic Gaussian point-cloud density and gradient so
  // density-normal flow can use the same field that emission/evolution code
  // will later expose.
  DensityFieldSample evaluateDensityField(const DensityField &field, double x, double y) {
    DensityFieldSample sample;
    if (!std::isfinite(field.kernelCovXX) || !std::isfinite(field.kernelCovXY) || !std::isfinite(field.kernelCovYY)) {
      return sample;
    }

    const double determinant = field.kernelCovXX * field.kernelCovYY - field.kernelCovXY * field.kernelCovXY;
    if (!std::isfinite(determinant) || determinant <= 0.0) {
      return sample;
    }

    const double inverseCovXX = field.kernelCovYY / determinant;
    const double inverseCovXY = -field.kernelCovXY / determinant;
    const double inverseCovYY = field.kernelCovXX / determinant;
    const double normalization = 1.0 / (2.0 * kPi * std::sqrt(determinant));

    for (const WeightedTransversePoint &point : field.supportPoints) {
      if (!std::isfinite(point.weight) || point.weight <= 0.0) {
        continue;
      }

      const double dx = x - point.x;
      const double dy = y - point.y;
      const double qx = inverseCovXX * dx + inverseCovXY * dy;
      const double qy = inverseCovXY * dx + inverseCovYY * dy;
      const double quadraticForm = dx * qx + dy * qy;
      if (!std::isfinite(quadraticForm)) {
        continue;
      }

      const double exponent = -0.5 * quadraticForm;
      const double contribution = point.weight * normalization * std::exp(exponent);
      sample.density += contribution;
      sample.gradientX -= contribution * qx;
      sample.gradientY -= contribution * qy;
    }

    return sample;
  }

}  // namespace blastwave
