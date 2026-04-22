#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "blastwave/BlastWaveGenerator.h"
#include "blastwave/FlowFieldModel.h"

namespace {

  constexpr double kPi = 3.14159265358979323846;
  constexpr double kTwoPi = 2.0 * kPi;

}  // namespace

namespace blastwave {

  // Sample the hotspot multiplicity from the configured NBD compound Poisson
  // model, returning zero directly when the mean intensity is disabled.
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

  // Smear the participant hotspot in the transverse plane while keeping the
  // unsmeared participant position available as source metadata.
  BlastWaveGenerator::SpatialPoint BlastWaveGenerator::smearSource(const Nucleon &participant) {
    if (config_.smearSigma <= 0.0) {
      return {participant.x, participant.y};
    }

    std::normal_distribution<double> smearDistribution(0.0, config_.smearSigma);
    return {participant.x + smearDistribution(rng_), participant.y + smearDistribution(rng_)};
  }

  // Sample the source space-time rapidity using the current plateau-plus-tail
  // convention so longitudinal geometry stays configurable but simple.
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

  // Sample a local-rest-frame momentum with a shared isotropic direction and a
  // magnitude supplied by the configured thermal model.
  BlastWaveGenerator::FourMomentum BlastWaveGenerator::sampleThermalMomentum() {
    if (config_.temperature <= 0.0) {
      return {0.0, 0.0, 0.0, config_.mass};
    }

    std::uniform_real_distribution<double> cosThetaDistribution(-1.0, 1.0);
    std::uniform_real_distribution<double> phiDistribution(0.0, kTwoPi);

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

  // Evaluate the default flow field from the event covariance ellipse so the
  // transverse direction follows the ellipse normal rather than the lab origin.
  BlastWaveGenerator::FlowVelocity BlastWaveGenerator::sampleFlowVelocity(const SpatialPoint &emissionPoint, double etaS, const FlowEllipseInfo &flowEllipse) const {
    const FlowFieldParameters parameters{config_.rho0, config_.rho2, config_.flowPower};
    const FlowFieldSample sample = evaluateFlowField(flowEllipse, emissionPoint.x, emissionPoint.y, parameters);
    const double coshEta = std::cosh(etaS);
    return {sample.betaX / coshEta, sample.betaY / coshEta, std::tanh(etaS)};
  }

  // Apply the full Lorentz boost from the local rest frame into the lab frame
  // after the blast-wave velocity field has been sampled.
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

}  // namespace blastwave
