#pragma once

#include <vector>

#include "blastwave/DensityFieldModel.h"

namespace blastwave {

  // Keep shared kinematic helpers in one interface so producer, validator,
  // and generator code agree on derived observables and simple mappings.
  [[nodiscard]] double computeCentralityPercent(double impactParameter, double woodsSaxonRadius);
  [[nodiscard]] double computeAzimuth(double px, double py);
  [[nodiscard]] double computePseudorapidity(double px, double py, double pz);
  [[nodiscard]] double computeSecondHarmonicEventV2(double q2x, double q2y, int multiplicity);
  // Keep harmonic moments available without ROOT by returning the recentered
  // event-plane observables used by generator and QA for cross-checks.
  struct RecenteredHarmonicGeometry {
    bool valid = false;
    double epsilon = 0.0;
    double psi = 0.0;
    double centerX = 0.0;
    double centerY = 0.0;
    double rRms = 0.0;
  };

  [[nodiscard]] RecenteredHarmonicGeometry computeRecenteredHarmonicGeometry(const std::vector<WeightedTransversePoint> &points, int harmonic);
  [[nodiscard]] double computeMeanRadiusSquared(const std::vector<TransversePoint> &points);
  [[nodiscard]] double computeMtCoshWeight(double mass, double px, double py, double pz, double etaS);

}  // namespace blastwave
