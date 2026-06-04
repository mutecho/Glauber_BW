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
  enum class InitialGeometryMode { Glauber, ResponseTest023 };
  enum class InitialGeometrySourceAllocationMode { RatioTotal, IndependentPools };

  struct BlastWaveConfig {
    int nEvents = 100;
    int nucleonsPerNucleus = 208;
    int pid = 211;
    int charge = 1;
    std::uint64_t seed = 12345;

    double impactParameter = 8.0;      // fm
    InitialGeometryMode initialGeometryMode = InitialGeometryMode::Glauber;
    InitialGeometrySourceAllocationMode initialGeometrySourceAllocationMode = InitialGeometrySourceAllocationMode::RatioTotal;
    int initialGeometrySourceCount = 600;
    bool initialGeometryFluctuate = false;
    int initialGeometrySourceCountMin = 600;
    int initialGeometrySourceCountMax = 600;
    double initialGeometryA2Min = 0.0;
    double initialGeometryA2Max = 0.0;
    double initialGeometryA3Min = 0.0;
    double initialGeometryA3Max = 0.0;
    double initialGeometryR2xMin = 1.8;
    double initialGeometryR2xMax = 1.8;
    double initialGeometryR2yMin = 1.8;
    double initialGeometryR2yMax = 1.8;
    double initialGeometryR3Min = 1.8;
    double initialGeometryR3Max = 1.8;
    double initialGeometrySigma3Min = 0.6;
    double initialGeometrySigma3Max = 0.6;
    bool hasInitialGeometrySourceCountMin = false;
    bool hasInitialGeometrySourceCountMax = false;
    bool hasInitialGeometryA2Min = false;
    bool hasInitialGeometryA2Max = false;
    bool hasInitialGeometryA3Min = false;
    bool hasInitialGeometryA3Max = false;
    bool hasInitialGeometryR2xMin = false;
    bool hasInitialGeometryR2xMax = false;
    bool hasInitialGeometryR2yMin = false;
    bool hasInitialGeometryR2yMax = false;
    bool hasInitialGeometryR3Min = false;
    bool hasInitialGeometryR3Max = false;
    bool hasInitialGeometrySigma3Min = false;
    bool hasInitialGeometrySigma3Max = false;
    double initialGeometryR0 = 1.2;
    double initialGeometryA2 = 0.0;
    double initialGeometryR2x = 1.8;
    double initialGeometryR2y = 1.8;
    double initialGeometryPhi2 = 0.0;
    double initialGeometryA3 = 0.0;
    double initialGeometryR3 = 1.8;
    double initialGeometrySigma3 = 0.6;
    double initialGeometryPhi3 = 0.0;
    bool debugInitialGeometry = false;
    double temperature = 0.2;          // GeV
    double tau0 = 10.0;                // fm/c
    double smearSigma = 0.5;           // fm
    double sigmaNN = 7.0;              // fm^2
    double sigmaEta = 1.5;             // Gaussian tail width in space-time rapidity
    double etaPlateauHalfWidth = 1.0;  // Flat |eta_s| core half width
    double nbdMu = 2.0;
    double nbdK = 1.5;
    double flowTransRho0 = 1.0986122886681098;
    double kappa2 = 1.0986122886681098;
    double flowTransProfilePower = 1.0;
    FlowTransMagnitudeMode flowTransMagnitudeMode = FlowTransMagnitudeMode::RadiusProfile;
    double flowTransGradientStrength = 0.0;
    double flowTransGradientDensityFloorFraction = 1.0e-4;
    double flowTransGradientMaxFactorDelta = 0.2;
    bool hasFlowTransMagnitudeMode = false;
    bool hasFlowTransGradientStrength = false;
    bool hasFlowTransGradientDensityFloorFraction = false;
    bool hasFlowTransGradientMaxFactorDelta = false;
    double flowTransDirectionGradientFraction = 1.0;
    FlowTransRadiusMode flowTransRadiusMode = FlowTransRadiusMode::Covariance;
    double flowTransRadiusFraction = 0.0;
    bool hasFlowTransDirectionGradientFraction = false;
    bool hasFlowTransRadius = false;
    FlowTransRadiusResolution flowTransRadiusResolution = FlowTransRadiusResolution::Balanced;
    bool hasFlowTransRadiusResolution = false;
    DensityEvolutionMode densityEvolutionMode = DensityEvolutionMode::AffineGaussianResponse;
    FlowVelocitySamplerMode flowVelocitySamplerMode = FlowVelocitySamplerMode::CovarianceEllipse;
    double flowDensitySigma = 0.5;     // fm
    double affineLambdaIn = 1.20;
    double affineLambdaOut = 1.05;
    double affineSigmaEvo = 0.5;       // fm
    double affineDeltaTauRef = 10.0;   // fm/c, effective affine closure timescale
    double affineKappaFlow = 10.0;
    double affineKappaAniso = 1.0;
    double affineUMax = 0.95;
    bool densityNormalKappaCompensation = false;
    bool debugFlowEllipse = false;
    double gradientSigmaEm = 0.0;
    double gradientSigmaDyn = 1.0;
    double gradientDensityFloorFraction = 1.0e-4;
    double gradientDensityCutoffFraction = 1.0e-6;
    double gradientDisplacementMax = 1.5;
    double gradientDisplacementKappa = 1.0;  // fm, makes kappa*|grad ln s| dimensionless
    double gradientDiffusionSigma = 0.0;
    double gradientVMax = 0.75;
    double gradientVelocityKappa = 1.0;      // fm, makes kappa*|grad ln s| dimensionless
    bool debugGradientResponse = false;
    CooperFryeWeightMode cooperFryeWeightMode = CooperFryeWeightMode::None;
    double woodsSaxonRadius = 6.62;        // fm
    double woodsSaxonDiffuseness = 0.546;  // fm
    double mass = 0.13957;                 // GeV
    ThermalSamplerMode thermalSamplerMode = ThermalSamplerMode::MaxwellJuttner;
    double mjPMax = 8.0;  // GeV
    int mjGridPoints = 4096;
    AffineEffectiveMode affineEffectiveMode = AffineEffectiveMode::AdditiveRho;
  };

  struct EventInfo {
    int eventId = 0;
    double impactParameter = 0.0;
    int nParticipants = 0;
    double eps2 = 0.0;
    double psi2 = 0.0;
    double eps2Freezeout = 0.0;
    double psi2Freezeout = 0.0;
    double chi2 = 0.0;
    double r2Initial = 0.0;
    double r2Final = 0.0;
    double r2Ratio = 0.0;
    double v2 = 0.0;
    double v2LabX = 0.0;
    double v2LabY = 0.0;
    double centrality = 0.0;
    int nCharged = 0;
    int initialGeometryMode = 0;
    double eps3 = 0.0;
    double psi3 = 0.0;
    double xCenterInitial = 0.0;
    double yCenterInitial = 0.0;
    double rRmsInitial = 0.0;
    double geoA2 = 0.0;
    double geoA3 = 0.0;
    double geoR2x = 0.0;
    double geoR2y = 0.0;
    double geoR3 = 0.0;
    double geoSigma3 = 0.0;
    double v3 = 0.0;
    double v2WrtPsi2 = 0.0;
    double v3WrtPsi3 = 0.0;
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
    double x0 = 0.0;
    double y0 = 0.0;
    double emissionWeight = 1.0;
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

    // Per-event sampled template geometry parameters for response-test-023.
    struct ResponseTest023EventGeometry {
      int sourceCount = 0;
      double a2 = 0.0;
      double a3 = 0.0;
      double r2x = 0.0;
      double r2y = 0.0;
      double r3 = 0.0;
      double sigma3 = 0.0;
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
    [[nodiscard]] std::vector<Nucleon> sampleGlauberParticipants();
    [[nodiscard]] std::vector<Nucleon> sampleResponseTest023Participants(const ResponseTest023EventGeometry &eventParameters);
    [[nodiscard]] ResponseTest023EventGeometry sampleResponseTest023EventGeometry();
    [[nodiscard]] Nucleon sampleSingleNucleon(double xShift, int nucleusId);
    [[nodiscard]] EventMedium buildMedium(const std::vector<Nucleon> &participants) const;
    [[nodiscard]] std::vector<EmissionSite> sampleEventEmissionSites(const EventMedium &medium);
    [[nodiscard]] double sampleEtaS();
    [[nodiscard]] FourMomentum sampleThermalMomentum();
    [[nodiscard]] FlowVelocity sampleFlowVelocity(const EmissionSite &emissionSite, double etaS, const EventMedium &medium) const;
    [[nodiscard]] FourMomentum lorentzBoost(const FourMomentum &localMomentum, const FlowVelocity &beta) const;
    void validateParticle(const ParticleRecord &particle) const;
    void validateConfig() const;

    BlastWaveConfig config_;
    std::optional<MaxwellJuttnerMomentumSampler> mjSampler_;
    std::mt19937_64 rng_;
    std::uniform_real_distribution<double> unitUniform_{0.0, 1.0};
  };

}  // namespace blastwave
