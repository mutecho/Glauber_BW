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

  // Keep the transverse-momentum azimuth convention shared between production
  // code and QA so event-plane observables are reconstructed consistently.
  double computeAzimuth(double px, double py) {
    return std::atan2(py, px);
  }

  // Convert Cartesian momentum components into pseudorapidity with a finite
  // fallback in the beam-aligned limit where the denominator becomes singular.
  double computePseudorapidity(double px, double py, double pz) {
    const double momentumMagnitude = std::sqrt(px * px + py * py + pz * pz);
    const double numerator = momentumMagnitude + pz;
    const double denominator = momentumMagnitude - pz;
    if (numerator <= 1.0e-9 || denominator <= 1.0e-9) {
      return (pz >= 0.0) ? 10.0 : -10.0;
    }

    return 0.5 * std::log(numerator / denominator);
  }

  // Interpret v2 as the magnitude of the second-harmonic event Q-vector used
  // by the historical analysis macro: |Q2| / M with M = event multiplicity.
  double computeSecondHarmonicEventV2(double q2x, double q2y, int multiplicity) {
    if (multiplicity <= 0) {
      return 0.0;
    }
    return std::hypot(q2x, q2y) / static_cast<double>(multiplicity);
  }

  // Compute the centered transverse source-size moment from finite points.
  double computeMeanRadiusSquared(const std::vector<TransversePoint> &points) {
    if (points.empty()) {
      return 0.0;
    }

    double sumX = 0.0;
    double sumY = 0.0;
    double sumR2 = 0.0;
    std::size_t finiteCount = 0;
    for (const TransversePoint &point : points) {
      if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
        continue;
      }
      sumX += point.x;
      sumY += point.y;
      sumR2 += point.x * point.x + point.y * point.y;
      ++finiteCount;
    }
    if (finiteCount == 0) {
      return 0.0;
    }
    const double inverseCount = 1.0 / static_cast<double>(finiteCount);
    const double meanX = sumX * inverseCount;
    const double meanY = sumY * inverseCount;
    return std::max(0.0, sumR2 * inverseCount - meanX * meanX - meanY * meanY);
  }

  // Provide an opt-in Cooper-Frye inspired weight proxy on tau=const slices.
  double computeMtCoshWeight(double mass, double px, double py, double pz, double etaS) {
    const double mT = std::sqrt(std::max(0.0, mass * mass + px * px + py * py));
    const double momentumRapidity = mT > 0.0 ? std::asinh(pz / mT) : 0.0;
    const double weight = mT * std::cosh(momentumRapidity - etaS);
    if (!std::isfinite(weight) || weight < 0.0) {
      return 0.0;
    }
    return weight;
  }

}  // namespace blastwave
