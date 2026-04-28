#include <cmath>
#include <stdexcept>

#include "blastwave/BlastWaveGenerator.h"

namespace {

  constexpr double kFiniteTolerance = 1.0e-5;

  bool isFinite(double value) {
    return std::isfinite(value);
  }

}  // namespace

namespace blastwave {

  // Validate the produced particle record before it reaches the ROOT output
  // layer so broken kinematics never become part of the serialized contract.
  void BlastWaveGenerator::validateParticle(const ParticleRecord &particle) const {
    const double fields[] = {
        particle.mass, particle.x, particle.y, particle.z, particle.t, particle.px, particle.py, particle.pz, particle.energy, particle.etaS, particle.sourceX, particle.sourceY,
        particle.x0, particle.y0, particle.emissionWeight};

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
    if (particle.emissionWeight < 0.0) {
      throw std::runtime_error("Generator produced a negative emission weight.");
    }
  }

  // Fail fast on configuration errors so the generator never starts with a
  // physically inconsistent or numerically unsafe parameter set.
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
    if (config_.woodsSaxonRadius <= 0.0 || config_.woodsSaxonDiffuseness <= 0.0) {
      throw std::invalid_argument("Woods-Saxon geometry parameters must be positive.");
    }
    if (config_.mass <= 0.0) {
      throw std::invalid_argument("particle mass must be positive.");
    }
    if (!isFinite(config_.rho0) || config_.rho0 < 0.0) {
      throw std::invalid_argument("rho0 must be finite and non-negative.");
    }
    if (!isFinite(config_.kappa2)) {
      throw std::invalid_argument("kappa2 must be finite.");
    }
    if (!isFinite(config_.flowPower) || config_.flowPower <= 0.0) {
      throw std::invalid_argument("flowPower must be finite and positive.");
    }
    if (!isFinite(config_.flowDensitySigma) || config_.flowDensitySigma <= 0.0) {
      throw std::invalid_argument("flowDensitySigma must be finite and positive.");
    }
    if (!isFinite(config_.gradientSigmaEm) || config_.gradientSigmaEm < 0.0) {
      throw std::invalid_argument("gradientSigmaEm must be finite and non-negative.");
    }
    if (!isFinite(config_.gradientSigmaDyn) || config_.gradientSigmaDyn < 0.0) {
      throw std::invalid_argument("gradientSigmaDyn must be finite and non-negative.");
    }
    if (!isFinite(config_.gradientDensityFloorFraction) || config_.gradientDensityFloorFraction < 0.0) {
      throw std::invalid_argument("gradientDensityFloorFraction must be finite and non-negative.");
    }
    if (!isFinite(config_.gradientDensityCutoffFraction) || config_.gradientDensityCutoffFraction < 0.0) {
      throw std::invalid_argument("gradientDensityCutoffFraction must be finite and non-negative.");
    }
    if (!isFinite(config_.gradientDisplacementMax) || config_.gradientDisplacementMax < 0.0) {
      throw std::invalid_argument("gradientDisplacementMax must be finite and non-negative.");
    }
    if (!isFinite(config_.gradientDisplacementKappa) || config_.gradientDisplacementKappa < 0.0) {
      throw std::invalid_argument("gradientDisplacementKappa must be finite and non-negative.");
    }
    if (!isFinite(config_.gradientDiffusionSigma) || config_.gradientDiffusionSigma < 0.0) {
      throw std::invalid_argument("gradientDiffusionSigma must be finite and non-negative.");
    }
    if (!isFinite(config_.gradientVMax) || config_.gradientVMax < 0.0 || config_.gradientVMax >= 1.0) {
      throw std::invalid_argument("gradientVMax must be finite and satisfy 0 <= gradientVMax < 1.");
    }
    if (!isFinite(config_.gradientVelocityKappa) || config_.gradientVelocityKappa < 0.0) {
      throw std::invalid_argument("gradientVelocityKappa must be finite and non-negative.");
    }
    if (config_.gradientSigmaDyn <= config_.gradientSigmaEm) {
      throw std::invalid_argument("gradientSigmaDyn must be greater than gradientSigmaEm.");
    }
    if (config_.densityEvolutionMode == DensityEvolutionMode::GradientResponse && config_.flowVelocitySamplerMode != FlowVelocitySamplerMode::GradientResponse) {
      throw std::invalid_argument("densityEvolutionMode=gradient-response requires flowVelocitySamplerMode=gradient-response.");
    }
    if (config_.flowVelocitySamplerMode == FlowVelocitySamplerMode::GradientResponse && config_.densityEvolutionMode != DensityEvolutionMode::GradientResponse) {
      throw std::invalid_argument("flowVelocitySamplerMode=gradient-response requires densityEvolutionMode=gradient-response.");
    }
  }

}  // namespace blastwave
