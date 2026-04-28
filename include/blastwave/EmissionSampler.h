#pragma once

#include <random>
#include <vector>

#include "blastwave/DensityFieldModel.h"
#include "blastwave/EventMedium.h"

namespace blastwave {

  enum class EmissionSamplerMode { ParticipantHotspot };

  /**
   * One sampled particle-emission site. position is the actual transverse
   * emission coordinate, while sourceAnchor preserves the legacy participant
   * hotspot metadata written to ROOT source_x/source_y.
   */
  struct EmissionSite {
    TransversePoint position;
    TransversePoint sourceAnchor;
  };

  /**
   * Parameters for selecting and configuring the transverse emission sampler.
   * Only the participant-hotspot backend is implemented now; the typed mode
   * keeps later density-field samplers from changing the generator loop.
   */
  struct EmissionParameters {
    EmissionSamplerMode mode = EmissionSamplerMode::ParticipantHotspot;
    double smearSigma = 0.5;  // fm
    double nbdMu = 2.0;
    double nbdK = 1.5;
  };

  [[nodiscard]] std::vector<EmissionSite> sampleEmissionSites(const EventMedium &medium, const EmissionParameters &parameters, std::mt19937_64 &rng);

}  // namespace blastwave
