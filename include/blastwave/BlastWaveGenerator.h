#pragma once

#include <cstdint>
#include <random>
#include <vector>

namespace blastwave {

struct BlastWaveConfig {
  int nEvents = 100;
  int nucleonsPerNucleus = 208;
  int pid = 211;
  int charge = 1;
  std::uint64_t seed = 12345;

  double impactParameter = 8.0;  // fm
  double temperature = 0.2;      // GeV
  double tau0 = 10.0;            // fm/c
  double smearSigma = 0.5;       // fm
  double sigmaNN = 7.0;          // fm^2
  double sigmaEta = 1.5;         // Gaussian tail width in space-time rapidity
  double etaPlateauHalfWidth = 1.0;  // Flat |eta_s| core half width
  double nbdMu = 2.0;
  double nbdK = 1.5;
  double vMax = 0.8;
  double referenceRadius = 6.0;  // fm
  double kappa2 = 0.5;
  double woodsSaxonRadius = 6.62;      // fm
  double woodsSaxonDiffuseness = 0.546;  // fm
  double mass = 0.13957;               // GeV
};

struct EventInfo {
  int eventId = 0;
  double impactParameter = 0.0;
  int nParticipants = 0;
  double eps2 = 0.0;
  double psi2 = 0.0;
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

struct GeneratedEvent {
  EventInfo info;
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
    bool participant = false;
  };

  struct SpatialPoint {
    double x = 0.0;
    double y = 0.0;
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

  [[nodiscard]] std::vector<Nucleon> sampleParticipants() ;
  [[nodiscard]] Nucleon sampleSingleNucleon(double xShift);
  [[nodiscard]] std::pair<double, double> computeParticipantShape(
      const std::vector<Nucleon>& participants) const;
  [[nodiscard]] int sampleMultiplicity();
  [[nodiscard]] SpatialPoint smearSource(const Nucleon& participant);
  [[nodiscard]] double sampleEtaS();
  [[nodiscard]] FourMomentum sampleThermalMomentum() ;
  [[nodiscard]] FlowVelocity sampleFlowVelocity(
      const SpatialPoint& emissionPoint,
      double etaS,
      double eps2,
      double psi2) const;
  [[nodiscard]] FourMomentum lorentzBoost(
      const FourMomentum& localMomentum,
      const FlowVelocity& beta) const;
  void validateParticle(const ParticleRecord& particle) const;
  void validateConfig() const;

  BlastWaveConfig config_;
  std::mt19937_64 rng_;
  std::uniform_real_distribution<double> unitUniform_{0.0, 1.0};
};

}  // namespace blastwave
