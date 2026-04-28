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

  struct KernelSampler2D {
    bool valid = false;
    double l11 = 0.0;
    double l21 = 0.0;
    double l22 = 0.0;
  };

  KernelSampler2D buildKernelSampler(const blastwave::DensityField &field) {
    KernelSampler2D sampler;
    if (!std::isfinite(field.kernelCovXX) || !std::isfinite(field.kernelCovXY) || !std::isfinite(field.kernelCovYY)) {
      return sampler;
    }
    if (field.kernelCovXX <= 0.0) {
      return sampler;
    }

    const double l11 = std::sqrt(field.kernelCovXX);
    const double l21 = field.kernelCovXY / l11;
    const double residual = field.kernelCovYY - l21 * l21;
    if (!std::isfinite(residual) || residual <= 0.0) {
      return sampler;
    }

    sampler.valid = true;
    sampler.l11 = l11;
    sampler.l21 = l21;
    sampler.l22 = std::sqrt(residual);
    return sampler;
  }

  blastwave::TransversePoint sampleKernelPosition(const blastwave::WeightedTransversePoint &center,
                                                  const KernelSampler2D &kernelSampler,
                                                  std::mt19937_64 &rng) {
    if (!kernelSampler.valid) {
      return {center.x, center.y};
    }

    std::normal_distribution<double> standardNormal(0.0, 1.0);
    const double z1 = standardNormal(rng);
    const double z2 = standardNormal(rng);
    const double dx = kernelSampler.l11 * z1;
    const double dy = kernelSampler.l21 * z1 + kernelSampler.l22 * z2;
    return {center.x + dx, center.y + dy};
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

  // Emit from the freeze-out density field while preserving participant source
  // anchors and the legacy per-participant multiplicity law.
  std::vector<blastwave::EmissionSite> sampleDensityFieldEmissionSites(const blastwave::EventMedium &medium,
                                                                       const blastwave::EmissionParameters &parameters,
                                                                       std::mt19937_64 &rng) {
    std::vector<blastwave::EmissionSite> sites;
    sites.reserve(medium.participantPoints.size() * 4U);

    const std::size_t supportSize = medium.emissionDensity.supportPoints.size();
    const KernelSampler2D kernelSampler = buildKernelSampler(medium.emissionDensity);
    for (std::size_t iParticipant = 0; iParticipant < medium.participantPoints.size(); ++iParticipant) {
      const blastwave::WeightedTransversePoint &participant = medium.participantPoints[iParticipant];
      if (!std::isfinite(participant.weight) || participant.weight <= 0.0) {
        continue;
      }

      const int multiplicity = sampleHotspotMultiplicity(parameters.nbdMu, parameters.nbdK, rng);
      const blastwave::WeightedTransversePoint &freezeoutCenter =
          iParticipant < supportSize ? medium.emissionDensity.supportPoints[iParticipant] : participant;
      for (int iParticle = 0; iParticle < multiplicity; ++iParticle) {
        blastwave::EmissionSite site;
        site.position = sampleKernelPosition(freezeoutCenter, kernelSampler, rng);
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
      case EmissionSamplerMode::DensityField:
        return sampleDensityFieldEmissionSites(medium, parameters, rng);
    }

    return {};
  }

}  // namespace blastwave
