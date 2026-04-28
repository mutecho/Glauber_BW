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
    return field;
  }

  // Evaluate the analytic Gaussian point-cloud density and gradient so
  // density-normal flow can use the same field that emission/evolution code
  // will later expose.
  DensityFieldSample evaluateDensityField(const DensityField &field, double x, double y) {
    DensityFieldSample sample;
    if (!std::isfinite(field.gaussianSigma) || field.gaussianSigma <= 0.0) {
      return sample;
    }

    const double sigma2 = field.gaussianSigma * field.gaussianSigma;
    const double inverseSigma2 = 1.0 / sigma2;
    const double normalization = 1.0 / (2.0 * kPi * sigma2);
    for (const WeightedTransversePoint &point : field.supportPoints) {
      if (!std::isfinite(point.weight) || point.weight <= 0.0) {
        continue;
      }

      const double dx = x - point.x;
      const double dy = y - point.y;
      const double exponent = -0.5 * (dx * dx + dy * dy) * inverseSigma2;
      const double contribution = point.weight * normalization * std::exp(exponent);
      sample.density += contribution;
      sample.gradientX -= contribution * dx * inverseSigma2;
      sample.gradientY -= contribution * dy * inverseSigma2;
    }

    return sample;
  }

}  // namespace blastwave
