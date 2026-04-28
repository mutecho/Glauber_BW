#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <vector>

#include "blastwave/EmissionSampler.h"
#include "blastwave/EventMedium.h"
#include "blastwave/MaxwellJuttnerMomentumSampler.h"

namespace blastwave {

  enum class ThermalSamplerMode { MaxwellJuttner, Gamma };

  struct BlastWaveConfig {
    int nEvents = 100;
    int nucleonsPerNucleus = 208;
    int pid = 211;
    int charge = 1;
    std::uint64_t seed = 12345;

    double impactParameter = 8.0;      // fm
    double temperature = 0.2;          // GeV
    double tau0 = 10.0;                // fm/c
    double smearSigma = 0.5;           // fm
    double sigmaNN = 7.0;              // fm^2
    double sigmaEta = 1.5;             // Gaussian tail width in space-time rapidity
    double etaPlateauHalfWidth = 1.0;  // Flat |eta_s| core half width
    double nbdMu = 2.0;
    double nbdK = 1.5;
    double rho0 = 1.0986122886681098;
    double rho2 = 1.0986122886681098;
    double flowPower = 1.0;
    FlowVelocitySamplerMode flowVelocitySamplerMode = FlowVelocitySamplerMode::CovarianceEllipse;
    double flowDensitySigma = 0.5;     // fm
    bool debugFlowEllipse = false;
    double woodsSaxonRadius = 6.62;        // fm
    double woodsSaxonDiffuseness = 0.546;  // fm
    double mass = 0.13957;                 // GeV
    ThermalSamplerMode thermalSamplerMode = ThermalSamplerMode::MaxwellJuttner;
    double mjPMax = 8.0;  // GeV
    int mjGridPoints = 4096;
  };

  struct EventInfo {
    int eventId = 0;
    double impactParameter = 0.0;
    int nParticipants = 0;
    double eps2 = 0.0;
    double psi2 = 0.0;
    double v2 = 0.0;
    double centrality = 0.0;
    int nCharged = 0;
  };

  struct ParticleRecord {
    int eventId = 0;
    int pid = 0;
    int charge = 0;
    double mass = 0.0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double t = 0.0;
    double px = 0.0;
    double py = 0.0;
    double pz = 0.0;
    double energy = 0.0;
    double etaS = 0.0;
    double sourceX = 0.0;
    double sourceY = 0.0;
  };

  struct ParticipantRecord {
    int eventId = 0;
    int nucleusId = 0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
  };

  struct GeneratedEvent {
    EventInfo info;
    EventMedium medium;
    std::vector<ParticipantRecord> participants;
    std::vector<ParticleRecord> particles;
  };

  class BlastWaveGenerator {
   public:
    explicit BlastWaveGenerator(BlastWaveConfig config);

    [[nodiscard]] GeneratedEvent generateEvent(int eventId);

   private:
    struct Nucleon {
      double x = 0.0;
      double y = 0.0;
      double z = 0.0;
      int nucleusId = 0;
      bool participant = false;
    };

    struct FourMomentum {
      double px = 0.0;
      double py = 0.0;
      double pz = 0.0;
      double energy = 0.0;
    };

    struct FlowVelocity {
      double bx = 0.0;
      double by = 0.0;
      double bz = 0.0;
    };

    [[nodiscard]] std::vector<Nucleon> sampleParticipants();
    [[nodiscard]] Nucleon sampleSingleNucleon(double xShift, int nucleusId);
    [[nodiscard]] EventMedium buildMedium(const std::vector<Nucleon> &participants) const;
    [[nodiscard]] std::vector<EmissionSite> sampleEventEmissionSites(const EventMedium &medium);
    [[nodiscard]] double sampleEtaS();
    [[nodiscard]] FourMomentum sampleThermalMomentum();
    [[nodiscard]] FlowVelocity sampleFlowVelocity(const TransversePoint &emissionPoint, double etaS, const EventMedium &medium) const;
    [[nodiscard]] FourMomentum lorentzBoost(const FourMomentum &localMomentum, const FlowVelocity &beta) const;
    void validateParticle(const ParticleRecord &particle) const;
    void validateConfig() const;

    BlastWaveConfig config_;
    std::optional<MaxwellJuttnerMomentumSampler> mjSampler_;
    std::mt19937_64 rng_;
    std::uniform_real_distribution<double> unitUniform_{0.0, 1.0};
  };

}  // namespace blastwave
