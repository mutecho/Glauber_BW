#include "blastwave/PhysicsUtils.h"

#include <algorithm>
#include <cmath>

namespace {

  // Keep interval clamping local to the utility implementation.
  double clampToRange(double value, double lower, double upper) {
    return std::max(lower, std::min(value, upper));
  }

}  // namespace

namespace blastwave {

  // Centrality is currently a fixed-b display mapping rather than a calibrated
  // minimum-bias observable, so the helper keeps that convention explicit.
  double computeCentralityPercent(double impactParameter, double woodsSaxonRadius) {
    return clampToRange(100.0 * impactParameter / (2.0 * woodsSaxonRadius), 0.0, 100.0);
  }

  // Convert Cartesian momentum components into pseudorapidity with a finite
  // fallback in the beam-aligned limit where the denominator becomes singular.
  double computePseudorapidity(double px, double py, double pz) {
    const double momentumMagnitude = std::sqrt(px * px + py * py + pz * pz);
    const double denominator = momentumMagnitude - pz;
    if (denominator <= 1.0e-9) {
      return (pz >= 0.0) ? 10.0 : -10.0;
    }

    return 0.5 * std::log((momentumMagnitude + pz) / denominator);
  }

}  // namespace blastwave
