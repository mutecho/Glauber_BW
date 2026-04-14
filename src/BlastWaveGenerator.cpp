#include "blastwave/BlastWaveGenerator.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace blastwave {

  namespace {

    constexpr double kPi = 3.14159265358979323846;
    constexpr double kTwoPi = 2.0 * kPi;
    constexpr double kFiniteTolerance = 1.0e-5;

    // 带范围限制的取值
    double clamp(double value, double lower, double upper) {
      return std::max(lower, std::min(value, upper));
    }

    bool isFinite(double value) {
      return std::isfinite(value);
    }

    double computeCentralityPercent(double impactParameter, double woodsSaxonRadius) {
      return clamp(100.0 * impactParameter / (2.0 * woodsSaxonRadius), 0.0, 100.0);
    }

  }  // namespace

  BlastWaveGenerator::BlastWaveGenerator(BlastWaveConfig config) : config_(std::move(config)), mjSampler_(), rng_(config_.seed) {
    validateConfig();
    if (config_.temperature > 0.0 && config_.thermalSamplerMode == ThermalSamplerMode::MaxwellJuttner) {
      mjSampler_.emplace(config_.mass, config_.temperature, config_.mjPMax, static_cast<std::size_t>(config_.mjGridPoints));
    }
  }

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

    for (const Nucleon &participant : participants) {
      ParticipantRecord participantRecord;
      participantRecord.eventId = eventId;
      participantRecord.nucleusId = participant.nucleusId;
      participantRecord.x = participant.x;
      participantRecord.y = participant.y;
      participantRecord.z = participant.z;
      event.participants.push_back(participantRecord);
    }

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

  // 输出一条part的vector（来自A和B核的都在此）
  std::vector<BlastWaveGenerator::Nucleon> BlastWaveGenerator::sampleParticipants() {
    std::vector<Nucleon> nucleusA;
    std::vector<Nucleon> nucleusB;
    nucleusA.reserve(static_cast<std::size_t>(config_.nucleonsPerNucleus));
    nucleusB.reserve(static_cast<std::size_t>(config_.nucleonsPerNucleus));

    const double halfImpactParameter = 0.5 * config_.impactParameter;
    for (int iNucleon = 0; iNucleon < config_.nucleonsPerNucleus; ++iNucleon) {
      nucleusA.push_back(sampleSingleNucleon(-halfImpactParameter, 0));
      nucleusB.push_back(sampleSingleNucleon(+halfImpactParameter, 1));
    }

    const double collisionDistanceSquared = config_.sigmaNN / kPi;
    for (Nucleon &nucleonA : nucleusA) {
      for (Nucleon &nucleonB : nucleusB) {
        const double dx = nucleonA.x - nucleonB.x;
        const double dy = nucleonA.y - nucleonB.y;
        const double transverseDistanceSquared = dx * dx + dy * dy;
        if (transverseDistanceSquared <= collisionDistanceSquared) {
          nucleonA.participant = true;
          nucleonB.participant = true;
        }
      }
    }

    std::vector<Nucleon> participants;
    participants.reserve(static_cast<std::size_t>(2 * config_.nucleonsPerNucleus));
    for (const Nucleon &nucleon : nucleusA) {
      if (nucleon.participant) {
        participants.push_back(nucleon);
      }
    }
    for (const Nucleon &nucleon : nucleusB) {
      if (nucleon.participant) {
        participants.push_back(nucleon);
      }
    }
    return participants;
  }

  // 使用Woods-Saxon抽样核子的r，并随机赋予球坐标角，得到xyz
  BlastWaveGenerator::Nucleon BlastWaveGenerator::sampleSingleNucleon(double xShift, int nucleusId) {
    const double radialMax = config_.woodsSaxonRadius + 10.0 * config_.woodsSaxonDiffuseness;
    std::uniform_real_distribution<double> cosThetaDistribution(-1.0, 1.0);
    std::uniform_real_distribution<double> phiDistribution(0.0, kTwoPi);

    while (true) {
      const double radius = radialMax * unitUniform_(rng_);
      const double density = 1.0 / (1.0 + std::exp((radius - config_.woodsSaxonRadius) / config_.woodsSaxonDiffuseness));
      const double acceptance = density * (radius * radius) / (radialMax * radialMax);
      if (unitUniform_(rng_) > acceptance) {
        continue;
      }

      const double cosTheta = cosThetaDistribution(rng_);
      const double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
      const double phi = phiDistribution(rng_);

      Nucleon nucleon;
      nucleon.x = radius * sinTheta * std::cos(phi) + xShift;
      nucleon.y = radius * sinTheta * std::sin(phi);
      nucleon.z = radius * cosTheta;
      nucleon.nucleusId = nucleusId;
      return nucleon;
    }
  }

  std::pair<double, double> BlastWaveGenerator::computeParticipantShape(const std::vector<Nucleon> &participants) const {
    if (participants.size() < 2U) {
      return {0.0, 0.0};
    }

    double meanX = 0.0;
    double meanY = 0.0;
    for (const Nucleon &participant : participants) {
      meanX += participant.x;
      meanY += participant.y;
    }
    const double inverseCount = 1.0 / static_cast<double>(participants.size());
    meanX *= inverseCount;
    meanY *= inverseCount;

    double sigmaX2 = 0.0;
    double sigmaY2 = 0.0;
    double sigmaXY = 0.0;
    for (const Nucleon &participant : participants) {
      const double dx = participant.x - meanX;
      const double dy = participant.y - meanY;
      sigmaX2 += dx * dx;
      sigmaY2 += dy * dy;
      sigmaXY += dx * dy;
    }
    sigmaX2 *= inverseCount;
    sigmaY2 *= inverseCount;
    sigmaXY *= inverseCount;

    const double denominator = sigmaX2 + sigmaY2;
    if (denominator <= std::numeric_limits<double>::epsilon()) {
      return {0.0, 0.0};
    }

    const double eccentricityX = sigmaY2 - sigmaX2;
    const double eccentricityY = 2.0 * sigmaXY;
    const double eps2 = std::sqrt(eccentricityX * eccentricityX + eccentricityY * eccentricityY) / denominator;
    const double psi2 = 0.5 * std::atan2(eccentricityY, eccentricityX);
    return {eps2, psi2};
  }

  int BlastWaveGenerator::sampleMultiplicity() {
    if (config_.nbdMu <= 0.0) {
      return 0;
    }

    const double gammaScale = config_.nbdMu / config_.nbdK;
    std::gamma_distribution<double> gammaDistribution(config_.nbdK, gammaScale);
    const double lambda = gammaDistribution(rng_);
    std::poisson_distribution<int> poissonDistribution(lambda);
    return poissonDistribution(rng_);
  }

  // 随机移动均值0，标准差smearSigma的正态分布
  BlastWaveGenerator::SpatialPoint BlastWaveGenerator::smearSource(const Nucleon &participant) {
    if (config_.smearSigma <= 0.0) {
      return {participant.x, participant.y};
    }

    std::normal_distribution<double> smearDistribution(0.0, config_.smearSigma);
    return {participant.x + smearDistribution(rng_), participant.y + smearDistribution(rng_)};
  }

  // 抽样eta
  double BlastWaveGenerator::sampleEtaS() {
    if (config_.etaPlateauHalfWidth <= 0.0 && config_.sigmaEta <= 0.0) {
      return 0.0;
    }
    if (config_.etaPlateauHalfWidth > 0.0 && config_.sigmaEta <= 0.0) {
      std::uniform_real_distribution<double> plateauDistribution(-config_.etaPlateauHalfWidth, config_.etaPlateauHalfWidth);
      return plateauDistribution(rng_);
    }
    if (config_.etaPlateauHalfWidth <= 0.0) {
      std::normal_distribution<double> etaDistribution(0.0, config_.sigmaEta);
      return etaDistribution(rng_);
    }

    // A Bjorken-like midrapidity core is approximated by a flat source density in
    // |eta_s| < eta0, with Gaussian tails outside the plateau to keep the
    // distribution continuous and normalizable.
    const double plateauArea = config_.etaPlateauHalfWidth;
    const double tailArea = config_.sigmaEta * std::sqrt(kPi / 2.0);
    const double plateauProbability = plateauArea / (plateauArea + tailArea);
    if (unitUniform_(rng_) < plateauProbability) {
      std::uniform_real_distribution<double> plateauDistribution(-config_.etaPlateauHalfWidth, config_.etaPlateauHalfWidth);
      return plateauDistribution(rng_);
    }

    std::normal_distribution<double> tailDistribution(0.0, config_.sigmaEta);
    const double sign = unitUniform_(rng_) < 0.5 ? -1.0 : 1.0;
    return sign * (config_.etaPlateauHalfWidth + std::abs(tailDistribution(rng_)));
  }

  BlastWaveGenerator::FourMomentum BlastWaveGenerator::sampleThermalMomentum() {
    if (config_.temperature <= 0.0) {
      return {0.0, 0.0, 0.0, config_.mass};
    }

    std::uniform_real_distribution<double> cosThetaDistribution(-1.0, 1.0);
    std::uniform_real_distribution<double> phiDistribution(0.0, kTwoPi);

    // Thermal mode only chooses the local-rest-frame |p|. Direction sampling
    // and on-shell reconstruction stay shared across MJ and legacy Gamma paths.
    double momentumMagnitude = 0.0;
    switch (config_.thermalSamplerMode) {
      case ThermalSamplerMode::MaxwellJuttner:
        if (!mjSampler_) {
          throw std::runtime_error("Maxwell-Juttner sampler table is not initialized.");
        }
        momentumMagnitude = mjSampler_->sample(unitUniform_(rng_));
        break;
      case ThermalSamplerMode::Gamma: {
        std::gamma_distribution<double> momentumMagnitudeDistribution(3.0, config_.temperature);
        momentumMagnitude = momentumMagnitudeDistribution(rng_);
        break;
      }
    }

    const double cosTheta = cosThetaDistribution(rng_);
    const double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
    const double phi = phiDistribution(rng_);

    const double px = momentumMagnitude * sinTheta * std::cos(phi);
    const double py = momentumMagnitude * sinTheta * std::sin(phi);
    const double pz = momentumMagnitude * cosTheta;
    const double energy = std::sqrt(config_.mass * config_.mass + momentumMagnitude * momentumMagnitude);
    return {px, py, pz, energy};
  }

  BlastWaveGenerator::FlowVelocity BlastWaveGenerator::sampleFlowVelocity(const SpatialPoint &emissionPoint, double etaS, double eps2, double psi2) const {
    const double radius = std::hypot(emissionPoint.x, emissionPoint.y);
    const double phi = std::atan2(emissionPoint.y, emissionPoint.x);
    const double modulation = 1.0 + 2.0 * config_.kappa2 * eps2 * std::cos(2.0 * (phi - psi2));
    const double profile = clamp((radius / config_.referenceRadius) * modulation, 0.0, 1.0);
    const double vT = clamp(config_.vMax * profile, 0.0, 0.95);
    const double transverseRapidity = std::atanh(vT);

    const double sinhRho = std::sinh(transverseRapidity);
    const double coshRho = std::cosh(transverseRapidity);
    const double sinhEta = std::sinh(etaS);
    const double coshEta = std::cosh(etaS);

    const double u0 = coshRho * coshEta;
    const double ux = sinhRho * std::cos(phi);
    const double uy = sinhRho * std::sin(phi);
    const double uz = coshRho * sinhEta;

    return {ux / u0, uy / u0, uz / u0};
  }

  BlastWaveGenerator::FourMomentum BlastWaveGenerator::lorentzBoost(const FourMomentum &localMomentum, const FlowVelocity &beta) const {
    const double betaSquared = beta.bx * beta.bx + beta.by * beta.by + beta.bz * beta.bz;
    if (betaSquared <= std::numeric_limits<double>::epsilon()) {
      return localMomentum;
    }
    if (betaSquared >= 1.0) {
      throw std::runtime_error("Flow field became superluminal.");
    }

    const double gamma = 1.0 / std::sqrt(1.0 - betaSquared);
    const double betaDotP = beta.bx * localMomentum.px + beta.by * localMomentum.py + beta.bz * localMomentum.pz;
    const double gammaFactor = (gamma - 1.0) / betaSquared;

    FourMomentum boostedMomentum;
    boostedMomentum.px = localMomentum.px + gammaFactor * betaDotP * beta.bx + gamma * beta.bx * localMomentum.energy;
    boostedMomentum.py = localMomentum.py + gammaFactor * betaDotP * beta.by + gamma * beta.by * localMomentum.energy;
    boostedMomentum.pz = localMomentum.pz + gammaFactor * betaDotP * beta.bz + gamma * beta.bz * localMomentum.energy;
    boostedMomentum.energy = gamma * (localMomentum.energy + betaDotP);
    return boostedMomentum;
  }

  void BlastWaveGenerator::validateParticle(const ParticleRecord &particle) const {
    const double fields[] = {
        particle.mass, particle.x, particle.y, particle.z, particle.t, particle.px, particle.py, particle.pz, particle.energy, particle.etaS, particle.sourceX, particle.sourceY};

    for (double field : fields) {
      if (!isFinite(field)) {
        throw std::runtime_error("Generator produced NaN or Inf in particle output.");
      }
    }

    const double massShell =
        particle.energy * particle.energy - (particle.px * particle.px + particle.py * particle.py + particle.pz * particle.pz) - particle.mass * particle.mass;
    if (std::abs(massShell) > kFiniteTolerance) {
      throw std::runtime_error("Generator produced an invalid on-shell particle.");
    }
  }

  void BlastWaveGenerator::validateConfig() const {
    if (config_.nEvents < 0) {
      throw std::invalid_argument("nEvents must be non-negative.");
    }
    if (config_.nucleonsPerNucleus <= 0) {
      throw std::invalid_argument("nucleonsPerNucleus must be positive.");
    }
    if (config_.mjPMax <= 0.0) {
      throw std::invalid_argument("mjPMax must be positive.");
    }
    if (config_.mjGridPoints < 2) {
      throw std::invalid_argument("mjGridPoints must be at least 2.");
    }
    if (config_.tau0 <= 0.0) {
      throw std::invalid_argument("tau0 must be positive.");
    }
    if (config_.sigmaNN <= 0.0) {
      throw std::invalid_argument("sigmaNN must be positive.");
    }
    if (config_.etaPlateauHalfWidth < 0.0) {
      throw std::invalid_argument("etaPlateauHalfWidth must be non-negative.");
    }
    if (config_.nbdK <= 0.0) {
      throw std::invalid_argument("nbdK must be positive.");
    }
    if (config_.referenceRadius <= 0.0) {
      throw std::invalid_argument("referenceRadius must be positive.");
    }
    if (config_.woodsSaxonRadius <= 0.0 || config_.woodsSaxonDiffuseness <= 0.0) {
      throw std::invalid_argument("Woods-Saxon geometry parameters must be positive.");
    }
    if (config_.mass <= 0.0) {
      throw std::invalid_argument("particle mass must be positive.");
    }
    if (config_.vMax < 0.0 || config_.vMax >= 1.0) {
      throw std::invalid_argument("vMax must stay in [0, 1).");
    }
  }

}  // namespace blastwave
