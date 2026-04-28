#pragma once

#include <vector>

#include "blastwave/DensityFieldModel.h"
#include "blastwave/FlowFieldModel.h"

namespace blastwave {

  enum class DensityEvolutionMode { None };

  /**
   * Parameters for constructing the event-level medium. The first supported
   * evolution mode is intentionally the identity map; future expansion models
   * should replace emissionDensity/emissionGeometry without changing the
   * participantGeometry contract used by event summaries.
   */
  struct EventMediumParameters {
    DensityEvolutionMode densityEvolutionMode = DensityEvolutionMode::None;
    double densitySigma = 0.5;  // fm
  };

  /**
   * Event-level medium state shared by emission sampling, flow sampling, and
   * optional serialization. participantGeometry preserves initial-state
   * observables, while emissionDensity/emissionGeometry are the stage future
   * density-evolution backends will modify before particle emission.
   */
  struct EventMedium {
    std::vector<WeightedTransversePoint> participantPoints;
    FlowEllipseInfo participantGeometry;
    DensityField initialDensity;
    DensityField emissionDensity;
    FlowEllipseInfo emissionGeometry;
  };

  [[nodiscard]] EventMedium buildEventMedium(const std::vector<WeightedTransversePoint> &points, const EventMediumParameters &parameters);

}  // namespace blastwave
