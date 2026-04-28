#include "blastwave/EmissionSampler.h"

#include <cmath>

namespace {

  int sampleHotspotMultiplicity(double nbdMu, double nbdK, std::mt19937_64 &rng) {
    if (nbdMu <= 0.0) {
      return 0;
    }

    const double gammaScale = nbdMu / nbdK;
    std::gamma_distribution<double> gammaDistribution(nbdK, gammaScale);
    const double lambda = gammaDistribution(rng);
    std::poisson_distribution<int> poissonDistribution(lambda);
    return poissonDistribution(rng);
  }

  blastwave::TransversePoint sampleSmearedPosition(const blastwave::WeightedTransversePoint &anchor, double smearSigma, std::mt19937_64 &rng) {
    if (smearSigma <= 0.0) {
      return {anchor.x, anchor.y};
    }

    std::normal_distribution<double> smearDistribution(0.0, smearSigma);
    return {anchor.x + smearDistribution(rng), anchor.y + smearDistribution(rng)};
  }

  // Preserve the current participant-hotspot emission law while moving it
  // behind an explicit sampler boundary that future density-field samplers can
  // replace without changing the generator's particle loop.
  std::vector<blastwave::EmissionSite> sampleParticipantHotspotEmissionSites(const blastwave::EventMedium &medium,
                                                                             const blastwave::EmissionParameters &parameters,
                                                                             std::mt19937_64 &rng) {
    std::vector<blastwave::EmissionSite> sites;
    sites.reserve(medium.participantPoints.size() * 4U);

    for (const blastwave::WeightedTransversePoint &participant : medium.participantPoints) {
      if (!std::isfinite(participant.weight) || participant.weight <= 0.0) {
        continue;
      }

      const int multiplicity = sampleHotspotMultiplicity(parameters.nbdMu, parameters.nbdK, rng);
      for (int iParticle = 0; iParticle < multiplicity; ++iParticle) {
        blastwave::EmissionSite site;
        site.position = sampleSmearedPosition(participant, parameters.smearSigma, rng);
        site.sourceAnchor = {participant.x, participant.y};
        sites.push_back(site);
      }
    }

    return sites;
  }

}  // namespace

namespace blastwave {

  std::vector<EmissionSite> sampleEmissionSites(const EventMedium &medium, const EmissionParameters &parameters, std::mt19937_64 &rng) {
    switch (parameters.mode) {
      case EmissionSamplerMode::ParticipantHotspot:
        return sampleParticipantHotspotEmissionSites(medium, parameters, rng);
    }

    return {};
  }

}  // namespace blastwave
