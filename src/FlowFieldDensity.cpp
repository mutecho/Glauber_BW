#include "blastwave/FlowFieldModel.h"

#include <cmath>

namespace {

  constexpr double kPi = 3.14159265358979323846;

}  // namespace

namespace blastwave {

  // Evaluate the smeared participant density and its analytic Gaussian
  // gradient so density-normal velocity sampling never falls back to
  // finite-difference noise.
  FlowDensitySample evaluateDensityField(const FlowFieldContext &context, double x, double y) {
    FlowDensitySample sample;
    if (!std::isfinite(context.flowDensitySigma) || context.flowDensitySigma <= 0.0) {
      return sample;
    }

    const double sigma2 = context.flowDensitySigma * context.flowDensitySigma;
    const double inverseSigma2 = 1.0 / sigma2;
    const double normalization = 1.0 / (2.0 * kPi * sigma2);
    for (const WeightedTransversePoint &point : context.participantPoints) {
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
