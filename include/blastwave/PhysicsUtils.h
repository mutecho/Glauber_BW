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
  [[nodiscard]] double computeMeanRadiusSquared(const std::vector<TransversePoint> &points);
  [[nodiscard]] double computeMtCoshWeight(double mass, double px, double py, double pz, double etaS);

}  // namespace blastwave
