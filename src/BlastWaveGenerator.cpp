#include "blastwave/BlastWaveGenerator.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "blastwave/FlowFieldModel.h"
#include "blastwave/PhysicsUtils.h"

namespace blastwave {

  BlastWaveGenerator::BlastWaveGenerator(BlastWaveConfig config) : config_(std::move(config)), mjSampler_(), rng_(config_.seed) {
    validateConfig();
    if (config_.temperature > 0.0 && config_.thermalSamplerMode == ThermalSamplerMode::MaxwellJuttner) {
      mjSampler_.emplace(config_.mass, config_.temperature, config_.mjPMax, static_cast<std::size_t>(config_.mjGridPoints));
    }
  }

  // Orchestrate one event by composing geometry, source sampling, flow, and
  // final record materialization without letting the app layer touch internals.
  GeneratedEvent BlastWaveGenerator::generateEvent(int eventId) {
    std::vector<Nucleon> participants = sampleParticipants();

    EventInfo info;
    info.eventId = eventId;
    info.impactParameter = config_.impactParameter;
    info.nParticipants = static_cast<int>(participants.size());

    const EventMedium medium = buildMedium(participants);
    info.eps2 = medium.participantGeometry.eps2;
    info.psi2 = medium.participantGeometry.psi2;
    info.centrality = computeCentralityPercent(config_.impactParameter, config_.woodsSaxonRadius);

    GeneratedEvent event;
    event.info = info;
    event.medium = medium;
    event.participants.reserve(participants.size());
    const std::vector<EmissionSite> emissionSites = sampleEventEmissionSites(event.medium);
    event.particles.reserve(emissionSites.size());
    double q2x = 0.0;
    double q2y = 0.0;
    double q2Weight = 0.0;
    std::vector<TransversePoint> initialEmissionPoints;
    std::vector<TransversePoint> finalEmissionPoints;
    initialEmissionPoints.reserve(emissionSites.size());
    finalEmissionPoints.reserve(emissionSites.size());
    for (const EmissionSite &emissionSite : emissionSites) {
      initialEmissionPoints.push_back(emissionSite.initialPosition);
      finalEmissionPoints.push_back(emissionSite.position);
    }

    // Track initial/final source-size moments from accepted emission sites.
    event.info.r2Initial = computeMeanRadiusSquared(initialEmissionPoints);
    event.info.r2Final = computeMeanRadiusSquared(finalEmissionPoints);
    event.info.r2Ratio = event.info.r2Initial > 1.0e-12 ? event.info.r2Final / event.info.r2Initial : 0.0;

    // Recover V2 freeze-out geometry from accepted final emission sites.
    if (config_.densityEvolutionMode == DensityEvolutionMode::GradientResponse) {
      std::vector<WeightedTransversePoint> freezeoutPoints;
      freezeoutPoints.reserve(finalEmissionPoints.size());
      for (const TransversePoint &point : finalEmissionPoints) {
        freezeoutPoints.push_back({point.x, point.y, 1.0});
      }
      event.medium.emissionGeometry = computeFlowEllipseInfo(freezeoutPoints);
    }
    event.info.eps2Freezeout = event.medium.emissionGeometry.eps2;
    event.info.psi2Freezeout = event.medium.emissionGeometry.psi2;
    event.info.chi2 = event.info.eps2 > 1.0e-12 ? event.info.eps2Freezeout / event.info.eps2 : 0.0;

    // Preserve an explicit participant record surface so downstream QA can
    // validate geometry independent of the particle emission loop.
    for (const Nucleon &participant : participants) {
      ParticipantRecord participantRecord;
      participantRecord.eventId = eventId;
      participantRecord.nucleusId = participant.nucleusId;
      participantRecord.x = participant.x;
      participantRecord.y = participant.y;
      participantRecord.z = participant.z;
      event.participants.push_back(participantRecord);
    }

    // Emit particles from sampler-provided sites so future density-field
    // backends can replace hotspot smearing without changing momentum/flow code.
    for (const EmissionSite &emissionSite : emissionSites) {
      const double etaS = sampleEtaS();
      const FlowVelocity beta = sampleFlowVelocity(emissionSite, etaS, event.medium);
      const FourMomentum localMomentum = sampleThermalMomentum();
      const FourMomentum boostedMomentum = lorentzBoost(localMomentum, beta);

      ParticleRecord particle;
      particle.eventId = eventId;
      particle.pid = config_.pid;
      particle.charge = config_.charge;
      particle.mass = config_.mass;
      particle.x = emissionSite.position.x;
      particle.y = emissionSite.position.y;
      particle.z = config_.tau0 * std::sinh(etaS);
      particle.t = config_.tau0 * std::cosh(etaS);
      particle.px = boostedMomentum.px;
      particle.py = boostedMomentum.py;
      particle.pz = boostedMomentum.pz;
      particle.energy = boostedMomentum.energy;
      particle.etaS = etaS;
      particle.sourceX = emissionSite.sourceAnchor.x;
      particle.sourceY = emissionSite.sourceAnchor.y;
      particle.x0 = emissionSite.initialPosition.x;
      particle.y0 = emissionSite.initialPosition.y;
      particle.emissionWeight = emissionSite.emissionWeight;
      if (config_.cooperFryeWeightMode == CooperFryeWeightMode::MtCosh) {
        particle.emissionWeight = particle.emissionWeight * computeMtCoshWeight(particle.mass, particle.px, particle.py, particle.pz, particle.etaS);
      }

      validateParticle(particle);
      // Accumulate the weighted final-state second-harmonic Q-vector while the
      // boosted momentum is still in hand so event summaries stay ROOT-free.
      const double phi = computeAzimuth(particle.px, particle.py);
      q2x += particle.emissionWeight * std::cos(2.0 * phi);
      q2y += particle.emissionWeight * std::sin(2.0 * phi);
      q2Weight += particle.emissionWeight;
      event.particles.push_back(particle);
    }

    event.info.nCharged = static_cast<int>(event.particles.size());
    event.info.v2 = q2Weight > 0.0 ? std::hypot(q2x, q2y) / q2Weight : 0.0;
    return event;
  }

}  // namespace blastwave
