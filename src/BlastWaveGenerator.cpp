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
    // Compute response-test template geometry once per event so geometry metadata
    // written into event info exactly matches the sampled source cloud.
    std::vector<Nucleon> participants;
    ResponseTest023EventGeometry responseEventGeometry;
    if (config_.initialGeometryMode == InitialGeometryMode::ResponseTest023) {
      responseEventGeometry = sampleResponseTest023EventGeometry();
      participants = sampleResponseTest023Participants(responseEventGeometry);
    } else {
      participants = sampleGlauberParticipants();
    }

    EventInfo info;
    info.eventId = eventId;
    info.impactParameter = config_.impactParameter;
    info.nParticipants = static_cast<int>(participants.size());
    info.initialGeometryMode = config_.initialGeometryMode == InitialGeometryMode::ResponseTest023 ? 1 : 0;
    if (config_.initialGeometryMode == InitialGeometryMode::ResponseTest023) {
      info.geoA2 = responseEventGeometry.a2;
      info.geoA3 = responseEventGeometry.a3;
      info.geoR2x = responseEventGeometry.r2x;
      info.geoR2y = responseEventGeometry.r2y;
      info.geoR3 = responseEventGeometry.r3;
      info.geoSigma3 = responseEventGeometry.sigma3;
    } else {
      info.geoA2 = config_.initialGeometryA2;
      info.geoA3 = config_.initialGeometryA3;
      info.geoR2x = config_.initialGeometryR2x;
      info.geoR2y = config_.initialGeometryR2y;
      info.geoR3 = config_.initialGeometryR3;
      info.geoSigma3 = config_.initialGeometrySigma3;
    }

    const EventMedium medium = buildMedium(participants);
    info.eps2 = medium.participantGeometry.eps2;
    info.psi2 = medium.participantGeometry.psi2;

    const RecenteredHarmonicGeometry initialGeometry = computeRecenteredHarmonicGeometry(medium.participantPoints, 3);
    if (initialGeometry.valid) {
      info.eps3 = initialGeometry.epsilon;
      info.psi3 = initialGeometry.psi;
      info.xCenterInitial = initialGeometry.centerX;
      info.yCenterInitial = initialGeometry.centerY;
      info.rRmsInitial = initialGeometry.rRms;
    }
    info.centrality = computeCentralityPercent(config_.impactParameter, config_.woodsSaxonRadius);

    GeneratedEvent event;
    event.info = info;
    event.medium = medium;
    event.participants.reserve(participants.size());
    const std::vector<EmissionSite> emissionSites = sampleEventEmissionSites(event.medium);
    event.particles.reserve(emissionSites.size());
    double q2x = 0.0;
    double q2y = 0.0;
    double q3x = 0.0;
    double q3y = 0.0;
    double harmonicWeight = 0.0;
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
      // Accumulate weighted final-state Q2/Q3 vectors while boosted momentum is
      // still in hand so event summaries stay ROOT-free.
      const double phi = computeAzimuth(particle.px, particle.py);
      q2x += particle.emissionWeight * std::cos(2.0 * phi);
      q2y += particle.emissionWeight * std::sin(2.0 * phi);
      q3x += particle.emissionWeight * std::cos(3.0 * phi);
      q3y += particle.emissionWeight * std::sin(3.0 * phi);
      harmonicWeight += particle.emissionWeight;
      event.particles.push_back(particle);
    }

    event.info.nCharged = static_cast<int>(event.particles.size());
    event.info.v2 = harmonicWeight > 0.0 ? std::hypot(q2x, q2y) / harmonicWeight : 0.0;
    event.info.v3 = harmonicWeight > 0.0 ? std::hypot(q3x, q3y) / harmonicWeight : 0.0;
    event.info.v2WrtPsi2 = harmonicWeight > 0.0 ? (q2x * std::cos(2.0 * event.info.psi2) + q2y * std::sin(2.0 * event.info.psi2)) / harmonicWeight : 0.0;
    event.info.v3WrtPsi3 =
        harmonicWeight > 0.0 ? (q3x * std::cos(3.0 * event.info.psi3) + q3y * std::sin(3.0 * event.info.psi3)) / harmonicWeight : 0.0;

    return event;
  }

}  // namespace blastwave
