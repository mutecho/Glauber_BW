#pragma once

#include <vector>

#include "blastwave/DensityFieldModel.h"
#include "blastwave/FlowFieldModel.h"

namespace blastwave {

  enum class DensityEvolutionMode { None, AffineGaussianResponse };

  /**
   * Parameters for constructing the event-level medium. Identity and affine
   * Gaussian response modes both preserve participantGeometry semantics while
   * allowing emissionDensity/emissionGeometry to represent freeze-out physics.
   */
  struct EventMediumParameters {
    DensityEvolutionMode densityEvolutionMode = DensityEvolutionMode::AffineGaussianResponse;
    double densitySigma = 0.5;  // fm
    double affineLambdaIn = 1.20;
    double affineLambdaOut = 1.05;
    double affineSigmaEvo = 0.5;  // fm
  };

  /**
   * Event-level medium state shared by emission sampling, flow sampling, and
   * optional serialization. participantGeometry preserves initial-state
   * observables, while emissionDensity/emissionGeometry are the stage future
   * density-evolution backends will modify before particle emission.
   */
  struct EventMedium {
    DensityEvolutionMode densityEvolutionMode = DensityEvolutionMode::None;
    std::vector<WeightedTransversePoint> participantPoints;
    FlowEllipseInfo participantGeometry;
    DensityField initialDensity;
    DensityField emissionDensity;
    FlowEllipseInfo emissionGeometry;
  };

  [[nodiscard]] EventMedium buildEventMedium(const std::vector<WeightedTransversePoint> &points, const EventMediumParameters &parameters);

}  // namespace blastwave
