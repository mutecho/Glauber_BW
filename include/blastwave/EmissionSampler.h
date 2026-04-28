#pragma once

#include <random>
#include <vector>

#include "blastwave/DensityFieldModel.h"
#include "blastwave/EventMedium.h"

namespace blastwave {

  enum class EmissionSamplerMode { ParticipantHotspot, DensityField, GradientResponse };
  enum class CooperFryeWeightMode { None, MtCosh };

  /**
   * One sampled particle-emission site. initialPosition captures the
   * pre-evolution transverse marker location while position stores the final
   * emission coordinate used by the generator.
   */
  struct EmissionSite {
    TransversePoint initialPosition;
    TransversePoint position;
    TransversePoint sourceAnchor;
    double gradientMagnitude = 0.0;
    double displacementX = 0.0;
    double displacementY = 0.0;
    double betaTX = 0.0;
    double betaTY = 0.0;
    double emissionWeight = 1.0;
  };

  /**
   * Parameters for selecting and configuring the transverse emission sampler.
   * Backend-specific knobs stay here so the generator loop only consumes
   * EmissionSite records regardless of the active medium response mode.
   */
  struct EmissionParameters {
    EmissionSamplerMode mode = EmissionSamplerMode::ParticipantHotspot;
    double smearSigma = 0.5;  // fm
    double nbdMu = 2.0;
    double nbdK = 1.5;
    double gradientDensityFloorFraction = 1.0e-4;
    double gradientDensityCutoffFraction = 1.0e-6;
    double gradientDisplacementMax = 1.5;
    double gradientDisplacementKappa = 1.0;  // fm, response scale for |grad ln s|
    double gradientDiffusionSigma = 0.0;
    double gradientVMax = 0.75;
    double gradientVelocityKappa = 1.0;      // fm, response scale for |grad ln s|
    CooperFryeWeightMode cooperFryeWeightMode = CooperFryeWeightMode::None;
  };

  [[nodiscard]] std::vector<EmissionSite> sampleEmissionSites(const EventMedium &medium, const EmissionParameters &parameters, std::mt19937_64 &rng);

}  // namespace blastwave
