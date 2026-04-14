#include "blastwave/BlastWaveGenerator.h"

#include <algorithm>
#include <utility>

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

    const auto [eps2, psi2] = computeParticipantShape(participants);
    info.eps2 = eps2;
    info.psi2 = psi2;
    info.centrality = computeCentralityPercent(config_.impactParameter, config_.woodsSaxonRadius);

    GeneratedEvent event;
    event.info = info;
    event.participants.reserve(participants.size());
    event.particles.reserve(static_cast<std::size_t>(std::max(0, info.nParticipants)) * 4U);

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

    // Emit particles by smearing each participant hotspot, sampling the local
    // rest-frame spectrum, and boosting into the lab frame through the flow field.
    for (const Nucleon &participant : participants) {
      const int multiplicity = sampleMultiplicity();
      for (int iParticle = 0; iParticle < multiplicity; ++iParticle) {
        const SpatialPoint emissionPoint = smearSource(participant);
        const double etaS = sampleEtaS();
        const FlowVelocity beta = sampleFlowVelocity(emissionPoint, etaS, info.eps2, info.psi2);
        const FourMomentum localMomentum = sampleThermalMomentum();
        const FourMomentum boostedMomentum = lorentzBoost(localMomentum, beta);

        ParticleRecord particle;
        particle.eventId = eventId;
        particle.pid = config_.pid;
        particle.charge = config_.charge;
        particle.mass = config_.mass;
        particle.x = emissionPoint.x;
        particle.y = emissionPoint.y;
        particle.z = config_.tau0 * std::sinh(etaS);
        particle.t = config_.tau0 * std::cosh(etaS);
        particle.px = boostedMomentum.px;
        particle.py = boostedMomentum.py;
        particle.pz = boostedMomentum.pz;
        particle.energy = boostedMomentum.energy;
        particle.etaS = etaS;
        particle.sourceX = participant.x;
        particle.sourceY = participant.y;

        validateParticle(particle);
        event.particles.push_back(particle);
      }
    }

    event.info.nCharged = static_cast<int>(event.particles.size());
    return event;
  }

}  // namespace blastwave
