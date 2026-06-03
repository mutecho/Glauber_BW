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
    if (config_.initialGeometryMode != InitialGeometryMode::Glauber && config_.initialGeometryMode != InitialGeometryMode::ResponseTest023) {
      throw std::invalid_argument("initial-geometry must be 'glauber' or 'response-test-023'.");
    }
    if (config_.initialGeometryFluctuate && config_.initialGeometryMode != InitialGeometryMode::ResponseTest023) {
      throw std::invalid_argument("initial-geometry-fluctuate requires initial-geometry=response-test-023.");
    }

    if (config_.initialGeometryFluctuate) {
      const bool hasAllSourceCountRange = config_.hasInitialGeometrySourceCountMin && config_.hasInitialGeometrySourceCountMax;
      const bool hasAllA2Range = config_.hasInitialGeometryA2Min && config_.hasInitialGeometryA2Max;
      const bool hasAllA3Range = config_.hasInitialGeometryA3Min && config_.hasInitialGeometryA3Max;
      const bool hasAllR2xRange = config_.hasInitialGeometryR2xMin && config_.hasInitialGeometryR2xMax;
      const bool hasAllR2yRange = config_.hasInitialGeometryR2yMin && config_.hasInitialGeometryR2yMax;
      const bool hasAllR3Range = config_.hasInitialGeometryR3Min && config_.hasInitialGeometryR3Max;
      const bool hasAllSigma3Range = config_.hasInitialGeometrySigma3Min && config_.hasInitialGeometrySigma3Max;
      if (!hasAllSourceCountRange || !hasAllA2Range || !hasAllA3Range || !hasAllR2xRange || !hasAllR2yRange || !hasAllR3Range
          || !hasAllSigma3Range) {
        throw std::invalid_argument(
            "initial-geometry-fluctuate=true requires all initial-geometry-* range options to be configured: source-count-min/max, a2-min/max, a3-min/max,"
            + std::string(" r2x-min/max, r2y-min/max, r3-min/max, sigma3-min/max."));
      }

      if (config_.initialGeometrySourceCountMin <= 0 || config_.initialGeometrySourceCountMax <= 0
          || config_.initialGeometrySourceCountMin > config_.initialGeometrySourceCountMax) {
        throw std::invalid_argument("initial-geometry-source-count range must be positive and satisfy min <= max.");
      }

      auto validateFiniteNonNegativeDoubleRange = [&](double minValue, double maxValue, const std::string &name, bool allowZero) {
        if (!isFinite(minValue) || !isFinite(maxValue) || minValue < 0.0 || (!allowZero && minValue <= 0.0)
            || (!allowZero && maxValue <= 0.0) || maxValue < minValue) {
          throw std::invalid_argument(name + " range must be finite with " + (allowZero ? "min >= 0" : "min > 0") + ", max > 0, and min <= max.");
        }
      };

      validateFiniteNonNegativeDoubleRange(config_.initialGeometryA2Min, config_.initialGeometryA2Max, "initial-geometry-a2", true);
      validateFiniteNonNegativeDoubleRange(config_.initialGeometryA3Min, config_.initialGeometryA3Max, "initial-geometry-a3", true);
      validateFiniteNonNegativeDoubleRange(config_.initialGeometryR2xMin, config_.initialGeometryR2xMax, "initial-geometry-r2x", false);
      validateFiniteNonNegativeDoubleRange(config_.initialGeometryR2yMin, config_.initialGeometryR2yMax, "initial-geometry-r2y", false);
      validateFiniteNonNegativeDoubleRange(config_.initialGeometryR3Min, config_.initialGeometryR3Max, "initial-geometry-r3", false);
      validateFiniteNonNegativeDoubleRange(config_.initialGeometrySigma3Min, config_.initialGeometrySigma3Max, "initial-geometry-sigma3", false);
    }

    if (config_.initialGeometrySourceCount <= 0) {
      throw std::invalid_argument("initial-geometry-source-count must be positive.");
    }
    if (!isFinite(config_.initialGeometryR0) || config_.initialGeometryR0 <= 0.0) {
      throw std::invalid_argument("initial-geometry-r0 must be finite and positive.");
    }
    if (!isFinite(config_.initialGeometryR2x) || config_.initialGeometryR2x <= 0.0) {
      throw std::invalid_argument("initial-geometry-r2x must be finite and positive.");
    }
    if (!isFinite(config_.initialGeometryR2y) || config_.initialGeometryR2y <= 0.0) {
      throw std::invalid_argument("initial-geometry-r2y must be finite and positive.");
    }
    if (!isFinite(config_.initialGeometryR3) || config_.initialGeometryR3 <= 0.0) {
      throw std::invalid_argument("initial-geometry-r3 must be finite and positive.");
    }
    if (!isFinite(config_.initialGeometrySigma3) || config_.initialGeometrySigma3 <= 0.0) {
      throw std::invalid_argument("initial-geometry-sigma3 must be finite and positive.");
    }
    if (!isFinite(config_.initialGeometryA2) || config_.initialGeometryA2 < 0.0) {
      throw std::invalid_argument("initial-geometry-a2 must be finite and non-negative.");
    }
    if (!isFinite(config_.initialGeometryA3) || config_.initialGeometryA3 < 0.0) {
      throw std::invalid_argument("initial-geometry-a3 must be finite and non-negative.");
    }
    if (!isFinite(config_.initialGeometryPhi2) || !isFinite(config_.initialGeometryPhi3)) {
      throw std::invalid_argument("initial-geometry phase angles must be finite.");
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
    if (!isFinite(config_.flowTransRho0) || config_.flowTransRho0 < 0.0) {
      throw std::invalid_argument("flow-trans-rho0 must be finite and non-negative.");
    }
    if (!isFinite(config_.kappa2)) {
      throw std::invalid_argument("kappa2 must be finite.");
    }
    if (!isFinite(config_.flowTransProfilePower) || config_.flowTransProfilePower <= 0.0) {
      throw std::invalid_argument("flow-trans-profile-power must be finite and positive.");
    }
    if (!isFinite(config_.flowTransDirectionGradientFraction) || config_.flowTransDirectionGradientFraction < 0.0 || config_.flowTransDirectionGradientFraction > 1.0) {
      throw std::invalid_argument("flow-trans-direction-gradient-fraction must be finite and satisfy 0 <= value <= 1.");
    }
    if (config_.flowTransRadiusMode == FlowTransRadiusMode::DensityPercentile) {
      if (!isFinite(config_.flowTransRadiusFraction) || config_.flowTransRadiusFraction <= 0.0 || config_.flowTransRadiusFraction >= 1.0) {
        throw std::invalid_argument("flow-trans-radius=density-percentile requires 0 < p < 1.");
      }
    } else if (config_.flowTransRadiusMode == FlowTransRadiusMode::DensityLevel) {
      if (!isFinite(config_.flowTransRadiusFraction) || config_.flowTransRadiusFraction <= 0.0 || config_.flowTransRadiusFraction >= 1.0) {
        throw std::invalid_argument("flow-trans-radius=density-level requires 0 < fraction < 1.");
      }
    }
    if (!isFinite(config_.flowDensitySigma) || config_.flowDensitySigma <= 0.0) {
      throw std::invalid_argument("flowDensitySigma must be finite and positive.");
    }
    if (!isFinite(config_.affineLambdaIn) || config_.affineLambdaIn <= 0.0) {
      throw std::invalid_argument("affineLambdaIn must be finite and positive.");
    }
    if (!isFinite(config_.affineLambdaOut) || config_.affineLambdaOut <= 0.0) {
      throw std::invalid_argument("affineLambdaOut must be finite and positive.");
    }
    if (!isFinite(config_.affineSigmaEvo) || config_.affineSigmaEvo < 0.0) {
      throw std::invalid_argument("affineSigmaEvo must be finite and non-negative.");
    }
    if (!isFinite(config_.affineDeltaTauRef) || config_.affineDeltaTauRef <= 0.0) {
      throw std::invalid_argument("affineDeltaTauRef must be finite and positive.");
    }
    if (!isFinite(config_.affineKappaFlow)) {
      throw std::invalid_argument("affineKappaFlow must be finite.");
    }
    if (!isFinite(config_.affineKappaAniso)) {
      throw std::invalid_argument("affineKappaAniso must be finite.");
    }
    if (!isFinite(config_.affineUMax) || config_.affineUMax <= 0.0 || config_.affineUMax >= 1.0) {
      throw std::invalid_argument("affineUMax must be finite and satisfy 0 < affineUMax < 1.");
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
    if (config_.flowVelocitySamplerMode == FlowVelocitySamplerMode::AffineEffective
        && config_.densityEvolutionMode != DensityEvolutionMode::AffineGaussianResponse) {
      throw std::invalid_argument("flowVelocitySamplerMode=affine-effective requires densityEvolutionMode=affine-gaussian.");
    }
    if (!std::isfinite(config_.flowTransGradientStrength)) {
      throw std::invalid_argument("flow-trans-gradient-strength must be finite.");
    }
    if (!std::isfinite(config_.flowTransGradientDensityFloorFraction) || config_.flowTransGradientDensityFloorFraction <= 0.0) {
      throw std::invalid_argument("flow-trans-gradient-density-floor-fraction must be finite and greater than 0.");
    }
    if (!std::isfinite(config_.flowTransGradientMaxFactorDelta) || config_.flowTransGradientMaxFactorDelta < 0.0
        || config_.flowTransGradientMaxFactorDelta >= 1.0) {
      throw std::invalid_argument("flow-trans-gradient-max-factor-delta must be finite and satisfy 0 <= value < 1.");
    }
    const bool hasExplicitFlowTransGradientParameters =
        config_.hasFlowTransGradientStrength || config_.hasFlowTransGradientDensityFloorFraction || config_.hasFlowTransGradientMaxFactorDelta;
    if (hasExplicitFlowTransGradientParameters
        && config_.flowTransMagnitudeMode != FlowTransMagnitudeMode::ShellGradientCorrected) {
      throw std::invalid_argument(
          "flow-trans-gradient-strength, flow-trans-gradient-density-floor-fraction, and flow-trans-gradient-max-factor-delta require flow-trans-magnitude-mode=shell-gradient-corrected.");
    }
    if (config_.flowTransMagnitudeMode == FlowTransMagnitudeMode::ShellGradientCorrected) {
      if (config_.flowVelocitySamplerMode != FlowVelocitySamplerMode::DensityNormal) {
        throw std::invalid_argument("flow-trans-magnitude-mode=shell-gradient-corrected requires flow-velocity-sampler=density-normal.");
      }
      if (config_.flowTransRadiusMode != FlowTransRadiusMode::DensityPercentile && config_.flowTransRadiusMode != FlowTransRadiusMode::DensityLevel) {
        throw std::invalid_argument("flow-trans-magnitude-mode=shell-gradient-corrected requires flow-trans-radius=density-percentile:<p> or density-level:<fraction>.");
      }
    }
    const bool hasDensityNormalOnlyFlowTransOverrides = config_.hasFlowTransDirectionGradientFraction || config_.hasFlowTransRadius
                                                     || config_.hasFlowTransRadiusResolution;
    if (hasDensityNormalOnlyFlowTransOverrides && config_.flowVelocitySamplerMode != FlowVelocitySamplerMode::DensityNormal) {
      throw std::invalid_argument(
          "flow-trans-direction-gradient-fraction, flow-trans-radius, and flow-trans-radius-resolution require flowVelocitySamplerMode=density-normal.");
    }
    if (config_.densityNormalKappaCompensation
        && (config_.densityEvolutionMode != DensityEvolutionMode::AffineGaussianResponse
            || config_.flowVelocitySamplerMode != FlowVelocitySamplerMode::DensityNormal)) {
      throw std::invalid_argument(
          "densityNormalKappaCompensation requires densityEvolutionMode=affine-gaussian and flowVelocitySamplerMode=density-normal.");
    }
  }

}  // namespace blastwave
