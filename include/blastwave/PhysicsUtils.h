#pragma once

namespace blastwave {

  // Keep shared kinematic helpers in one interface so producer, validator,
  // and generator code agree on derived observables and simple mappings.
  [[nodiscard]] double computeCentralityPercent(double impactParameter, double woodsSaxonRadius);
  [[nodiscard]] double computePseudorapidity(double px, double py, double pz);

}  // namespace blastwave
