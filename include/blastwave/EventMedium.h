#pragma once

#include <vector>

#include "blastwave/DensityFieldModel.h"
#include "blastwave/FlowFieldModel.h"

namespace blastwave {

  enum class DensityEvolutionMode { None, AffineGaussianResponse, GradientResponse };

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
    double gradientSigmaEm = 0.0;              // fm
    double gradientSigmaDyn = 1.0;             // fm
    double gradientDensityFloorFraction = 1.0e-4;
    double gradientDensityCutoffFraction = 1.0e-6;
  };

  /**
   * Event-level affine-effective closure recovered from the initial and
   * freeze-out covariance semi-axes. The in-plane direction follows the
   * deterministic minor-axis convention shared by psi2 and flow sampling.
   */
  struct AffineEffectiveClosure {
    bool valid = false;
    double psi2 = 0.0;
    double sigmaInInitial = 0.0;
    double sigmaOutInitial = 0.0;
    double sigmaInFinal = 0.0;
    double sigmaOutFinal = 0.0;
    double growthIn = 0.0;
    double growthOut = 0.0;
    double lambdaIn = 0.0;
    double lambdaOut = 0.0;
    double lambdaBar = 0.0;
    double deltaLambda = 0.0;
  };

  /**
   * Cached angular boundary profile for density-defined transverse-flow
   * radii. The mode/fraction pair is part of the cache key because percentile
   * and level radii have different physical meanings.
   */
  struct FlowTransRadiusProfile {
    bool valid = false;
    FlowTransRadiusMode mode = FlowTransRadiusMode::Covariance;
    double fraction = 0.0;
    double centerX = 0.0;
    double centerY = 0.0;
    double radiusUpperBound = 0.0;
    int angularSamples = 0;
    std::vector<double> boundaryRadii;
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
    DensityField markerDensity;
    DensityField dynamicsDensity;
    double markerDensityScale = 0.0;
    double dynamicsDensityScale = 0.0;
    DensityField emissionDensity;
    double emissionDensityScale = 0.0;
    FlowEllipseInfo emissionGeometry;
    AffineEffectiveClosure affineEffectiveClosure;
    mutable FlowTransRadiusProfile flowTransRadiusProfile;
  };

  [[nodiscard]] EventMedium buildEventMedium(const std::vector<WeightedTransversePoint> &points, const EventMediumParameters &parameters);

}  // namespace blastwave
