#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "blastwave/BlastWaveGenerator.h"
#include "generate_blastwave/RunOptions.h"

namespace {

  void require(bool condition, const std::string &message) {
    if (!condition) {
      throw std::runtime_error(message);
    }
  }

  void requireNear(double actual, double expected, double tolerance, const std::string &message) {
    if (std::abs(actual - expected) > tolerance) {
      throw std::runtime_error(message);
    }
  }

  class TemporaryConfigFile {
   public:
    explicit TemporaryConfigFile(const std::string &contents) : path_(std::filesystem::temp_directory_path() / "blastwave_run_options_test.cfg") {
      std::ofstream output(path_);
      if (!output.is_open()) {
        throw std::runtime_error("Failed to create temporary config file.");
      }
      output << contents;
    }

    ~TemporaryConfigFile() {
      std::error_code error;
      std::filesystem::remove(path_, error);
    }

    [[nodiscard]] const std::filesystem::path &path() const {
      return path_;
    }

   private:
    std::filesystem::path path_;
  };

  struct ParsedRunOptions {
    blastwave::app::RunOptions runOptions;
    bool showHelp = false;
  };

  void requireFlowTransMagnitudeMode(const blastwave::BlastWaveConfig &config, blastwave::FlowTransMagnitudeMode expected, const std::string &message) {
    require(config.flowTransMagnitudeMode == expected, message);
  }

  void requireHasFlowTransMagnitudeMode(const blastwave::BlastWaveConfig &config, const std::string &message) {
    require(config.hasFlowTransMagnitudeMode, message);
  }

  ParsedRunOptions parseArguments(const std::vector<std::string> &arguments) {
    std::vector<std::string> storage;
    storage.reserve(arguments.size() + 1U);
    storage.emplace_back("generate_blastwave_events");
    storage.insert(storage.end(), arguments.begin(), arguments.end());

    std::vector<char *> argv;
    argv.reserve(storage.size());
    for (std::string &argument : storage) {
      argv.push_back(argument.data());
    }

    ParsedRunOptions parsed;
    parsed.runOptions = blastwave::app::parseRunOptions(static_cast<int>(argv.size()), argv.data(), parsed.showHelp);
    return parsed;
  }

  blastwave::BlastWaveConfig makeShellGradientConfig(blastwave::FlowTransRadiusMode radiusMode,
                                                     double radiusFraction,
                                                     double gradientStrength,
                                                     double floorFraction,
                                                     double maxFactorDelta,
                                                     blastwave::FlowVelocitySamplerMode samplerMode = blastwave::FlowVelocitySamplerMode::DensityNormal) {
    blastwave::BlastWaveConfig config;
    config.flowVelocitySamplerMode = samplerMode;
    config.densityEvolutionMode = blastwave::DensityEvolutionMode::AffineGaussianResponse;
    config.flowTransRadiusMode = radiusMode;
    config.flowTransRadiusFraction = radiusFraction;
    config.hasFlowTransRadius = true;
    config.flowTransMagnitudeMode = blastwave::FlowTransMagnitudeMode::ShellGradientCorrected;
    config.hasFlowTransMagnitudeMode = true;
    config.flowTransGradientStrength = gradientStrength;
    config.flowTransGradientDensityFloorFraction = floorFraction;
    config.flowTransGradientMaxFactorDelta = maxFactorDelta;
    config.hasFlowTransGradientStrength = true;
    config.hasFlowTransGradientDensityFloorFraction = true;
    config.hasFlowTransGradientMaxFactorDelta = true;
    return config;
  }

  void requireGeneratorValidationSuccess(const blastwave::BlastWaveConfig &config, const std::string &message) {
    try {
      blastwave::BlastWaveGenerator generator(config);
      static_cast<void>(generator);
    } catch (const std::exception &error) {
      throw std::runtime_error(message + " error=" + error.what());
    }
  }

  void requireGeneratorValidationFailure(const blastwave::BlastWaveConfig &config, const std::string &message) {
    bool threw = false;
    try {
      blastwave::BlastWaveGenerator generator(config);
      static_cast<void>(generator);
    } catch (const std::invalid_argument &) {
      threw = true;
    }

    require(threw, message);
  }

  void requireGeneratorValidationFailureContains(const blastwave::BlastWaveConfig &config,
                                                const std::string &expectedText,
                                                const std::string &message) {
    bool threw = false;
    try {
      blastwave::BlastWaveGenerator generator(config);
      static_cast<void>(generator);
    } catch (const std::invalid_argument &error) {
      threw = std::string(error.what()).find(expectedText) != std::string::npos;
    }
    require(threw, message);
  }

  // Build a complete fluctuating response-test config for validation matrix cases.
  blastwave::BlastWaveConfig makeValidFluctuating023Config() {
    blastwave::BlastWaveConfig config;
    config.initialGeometryMode = blastwave::InitialGeometryMode::ResponseTest023;
    config.initialGeometryFluctuate = true;
    config.initialGeometrySourceCountMin = 11;
    config.initialGeometrySourceCountMax = 17;
    config.initialGeometryA2Min = 0.1;
    config.initialGeometryA2Max = 0.8;
    config.initialGeometryA3Min = 0.2;
    config.initialGeometryA3Max = 0.6;
    config.initialGeometryR2xMin = 1.8;
    config.initialGeometryR2xMax = 3.2;
    config.initialGeometryR2yMin = 0.9;
    config.initialGeometryR2yMax = 1.8;
    config.initialGeometryR3Min = 1.4;
    config.initialGeometryR3Max = 3.0;
    config.initialGeometrySigma3Min = 0.45;
    config.initialGeometrySigma3Max = 1.3;
    config.hasInitialGeometrySourceCountMin = true;
    config.hasInitialGeometrySourceCountMax = true;
    config.hasInitialGeometryA2Min = true;
    config.hasInitialGeometryA2Max = true;
    config.hasInitialGeometryA3Min = true;
    config.hasInitialGeometryA3Max = true;
    config.hasInitialGeometryR2xMin = true;
    config.hasInitialGeometryR2xMax = true;
    config.hasInitialGeometryR2yMin = true;
    config.hasInitialGeometryR2yMax = true;
    config.hasInitialGeometryR3Min = true;
    config.hasInitialGeometryR3Max = true;
    config.hasInitialGeometrySigma3Min = true;
    config.hasInitialGeometrySigma3Max = true;
    return config;
  }

  // Check the full response-test fluctuation range contract after parsing.
  void requireFluctuatingRangeValues(const blastwave::BlastWaveConfig &config, const std::string &messagePrefix) {
    require(config.initialGeometryFluctuate, messagePrefix + " should enable initial-geometry-fluctuate.");
    require(config.hasInitialGeometrySourceCountMin && config.hasInitialGeometrySourceCountMax,
            messagePrefix + " should mark source-count range explicit.");
    require(config.hasInitialGeometryA2Min && config.hasInitialGeometryA2Max, messagePrefix + " should mark a2 range explicit.");
    require(config.hasInitialGeometryA3Min && config.hasInitialGeometryA3Max, messagePrefix + " should mark a3 range explicit.");
    require(config.hasInitialGeometryR2xMin && config.hasInitialGeometryR2xMax, messagePrefix + " should mark r2x range explicit.");
    require(config.hasInitialGeometryR2yMin && config.hasInitialGeometryR2yMax, messagePrefix + " should mark r2y range explicit.");
    require(config.hasInitialGeometryR3Min && config.hasInitialGeometryR3Max, messagePrefix + " should mark r3 range explicit.");
    require(config.hasInitialGeometrySigma3Min && config.hasInitialGeometrySigma3Max, messagePrefix + " should mark sigma3 range explicit.");
    require(config.initialGeometrySourceCountMin == 180 && config.initialGeometrySourceCountMax == 700,
            messagePrefix + " source-count range mismatch.");
    requireNear(config.initialGeometryA2Min, 0.0, 1.0e-12, messagePrefix + " a2-min mismatch.");
    requireNear(config.initialGeometryA2Max, 1.0, 1.0e-12, messagePrefix + " a2-max mismatch.");
    requireNear(config.initialGeometryA3Min, 0.0, 1.0e-12, messagePrefix + " a3-min mismatch.");
    requireNear(config.initialGeometryA3Max, 0.7, 1.0e-12, messagePrefix + " a3-max mismatch.");
    requireNear(config.initialGeometryR2xMin, 1.8, 1.0e-12, messagePrefix + " r2x-min mismatch.");
    requireNear(config.initialGeometryR2xMax, 3.2, 1.0e-12, messagePrefix + " r2x-max mismatch.");
    requireNear(config.initialGeometryR2yMin, 0.9, 1.0e-12, messagePrefix + " r2y-min mismatch.");
    requireNear(config.initialGeometryR2yMax, 1.8, 1.0e-12, messagePrefix + " r2y-max mismatch.");
    requireNear(config.initialGeometryR3Min, 1.4, 1.0e-12, messagePrefix + " r3-min mismatch.");
    requireNear(config.initialGeometryR3Max, 3.0, 1.0e-12, messagePrefix + " r3-max mismatch.");
    requireNear(config.initialGeometrySigma3Min, 0.45, 1.0e-12, messagePrefix + " sigma3-min mismatch.");
    requireNear(config.initialGeometrySigma3Max, 1.3, 1.0e-12, messagePrefix + " sigma3-max mismatch.");
  }

  void requireParseFailureContains(const std::vector<std::string> &arguments, const std::string &expectedText, const std::string &message) {
    bool threw = false;
    try {
      static_cast<void>(parseArguments(arguments));
    } catch (const std::invalid_argument &error) {
      threw = std::string(error.what()).find(expectedText) != std::string::npos;
    }
    require(threw, message);
  }

  void runDefaultSamplerTest() {
    const ParsedRunOptions parsed = parseArguments({});
    require(!parsed.showHelp, "Default parse should not request help.");
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
            "Default flow velocity sampler should stay covariance-ellipse.");
    require(parsed.runOptions.config.flowTransRadiusResolution == blastwave::FlowTransRadiusResolution::Balanced,
            "Default flow-trans-radius-resolution should stay balanced.");
    require(!parsed.runOptions.config.hasFlowTransRadiusResolution,
            "Default flow-trans-radius-resolution should stay implicit.");
    require(parsed.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse,
            "Default density evolution mode should stay affine-gaussian.");
    requireNear(parsed.runOptions.config.flowTransRho0, 1.0986122886681098, 1.0e-12, "Default flow-trans-rho0 should preserve the previous rho0 numeric default.");
    requireNear(parsed.runOptions.config.kappa2, 1.0986122886681098, 1.0e-12, "Default kappa2 should preserve the previous rho2 numeric default.");
    requireNear(parsed.runOptions.config.flowTransProfilePower, 1.0, 1.0e-12, "Default flow-trans-profile-power should stay 1.");
    requireNear(parsed.runOptions.config.flowTransDirectionGradientFraction, 1.0, 1.0e-12, "Default flow-trans direction fraction should stay density-gradient.");
    require(parsed.runOptions.config.flowTransRadiusMode == blastwave::FlowTransRadiusMode::Covariance, "Default flow-trans radius should stay covariance.");
    requireNear(parsed.runOptions.config.flowDensitySigma, 0.5, 1.0e-12, "Default flow density sigma should stay 0.5 fm.");
    requireNear(parsed.runOptions.config.affineLambdaIn, 1.20, 1.0e-12, "Default affine lambda in should stay 1.20.");
    requireNear(parsed.runOptions.config.affineLambdaOut, 1.05, 1.0e-12, "Default affine lambda out should stay 1.05.");
    requireNear(parsed.runOptions.config.affineSigmaEvo, 0.5, 1.0e-12, "Default affine sigma evo should stay 0.5 fm.");
    requireNear(parsed.runOptions.config.affineDeltaTauRef, 10.0, 1.0e-12, "Default affine delta-tau-ref should stay 10.0 fm/c.");
    requireNear(parsed.runOptions.config.affineKappaFlow, 10.0, 1.0e-12, "Default affine kappa-flow should stay 10.0.");
    requireNear(parsed.runOptions.config.affineKappaAniso, 1.0, 1.0e-12, "Default affine kappa-aniso should stay 1.0.");
    requireNear(parsed.runOptions.config.affineUMax, 0.95, 1.0e-12, "Default affine-u-max should stay 0.95.");
    require(parsed.runOptions.config.affineEffectiveMode == blastwave::AffineEffectiveMode::AdditiveRho,
            "Default affine-effective-mode should be additive-rho.");
    requireNear(parsed.runOptions.config.gradientSigmaEm, 0.0, 1.0e-12, "Default gradient sigma em should stay disabled.");
    requireNear(parsed.runOptions.config.gradientSigmaDyn, 1.0, 1.0e-12, "Default gradient sigma dyn should stay 1.0 fm.");
    requireNear(parsed.runOptions.config.gradientDensityFloorFraction, 1.0e-4, 1.0e-16, "Default gradient density floor fraction mismatch.");
    requireNear(parsed.runOptions.config.gradientDensityCutoffFraction, 1.0e-6, 1.0e-18, "Default gradient density cutoff fraction mismatch.");
    requireNear(parsed.runOptions.config.gradientDisplacementMax, 1.5, 1.0e-12, "Default gradient displacement max mismatch.");
    requireNear(parsed.runOptions.config.gradientDisplacementKappa, 1.0, 1.0e-12, "Default gradient displacement kappa mismatch.");
    requireNear(parsed.runOptions.config.gradientDiffusionSigma, 0.0, 1.0e-12, "Default gradient diffusion sigma should stay disabled.");
    requireNear(parsed.runOptions.config.gradientVMax, 0.75, 1.0e-12, "Default gradient vmax mismatch.");
    requireNear(parsed.runOptions.config.gradientVelocityKappa, 1.0, 1.0e-12, "Default gradient velocity kappa mismatch.");
    require(!parsed.runOptions.config.densityNormalKappaCompensation, "Density-normal kappa compensation should default to false.");
    require(!parsed.runOptions.config.debugGradientResponse, "Debug gradient response should stay disabled by default.");
    require(parsed.runOptions.config.cooperFryeWeightMode == blastwave::CooperFryeWeightMode::None,
            "Cooper-Frye weight should default to none.");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "Default covariance sampler should validate with implicit balanced radius resolution.");
  }

  void runFlowTransCliParseTest() {
    const ParsedRunOptions parsed = parseArguments({"--flow-velocity-sampler",
                                                    "density-normal",
                                                    "--density-evolution",
                                                    "affine-gaussian",
                                                    "--flow-trans-rho0",
                                                    "0.83",
                                                    "--flow-trans-profile-power",
                                                    "1.4",
                                                    "--flow-trans-direction-gradient-fraction",
                                                    "0.25",
                                                    "--flow-trans-radius",
                                                    "covariance",
                                                    "--kappa2",
                                                    "0.06"});
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::DensityNormal,
            "CLI should accept density-normal with flow-trans controls.");
    requireNear(parsed.runOptions.config.flowTransRho0, 0.83, 1.0e-12, "CLI flow-trans-rho0 parse mismatch.");
    requireNear(parsed.runOptions.config.flowTransProfilePower, 1.4, 1.0e-12, "CLI flow-trans-profile-power parse mismatch.");
    requireNear(parsed.runOptions.config.flowTransDirectionGradientFraction, 0.25, 1.0e-12, "CLI flow-trans direction fraction parse mismatch.");
    require(parsed.runOptions.config.flowTransRadiusMode == blastwave::FlowTransRadiusMode::Covariance, "CLI flow-trans-radius covariance parse mismatch.");
    requireNear(parsed.runOptions.config.kappa2, 0.06, 1.0e-12, "CLI kappa2 parse should remain unchanged.");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "Density-normal flow-trans CLI config should validate.");
  }

  void runFlowTransConfigAndOverrideParseTest() {
    const TemporaryConfigFile configFile(
        "flow-velocity-sampler = density-normal\n"
        "density-evolution = affine-gaussian\n"
        "flow-trans-rho0 = 0.91\n"
        "flow-trans-profile-power = 1.2\n"
        "flow-trans-direction-gradient-fraction = 1.0\n"
        "flow-trans-radius = density-level:1.0e-3\n");
    const ParsedRunOptions configOnly = parseArguments({configFile.path().string()});
    requireNear(configOnly.runOptions.config.flowTransRho0, 0.91, 1.0e-12, "Config flow-trans-rho0 parse mismatch.");
    requireNear(configOnly.runOptions.config.flowTransProfilePower, 1.2, 1.0e-12, "Config flow-trans-profile-power parse mismatch.");
    require(configOnly.runOptions.config.flowTransRadiusMode == blastwave::FlowTransRadiusMode::DensityLevel,
            "Config flow-trans-radius should parse density-level.");
    requireNear(configOnly.runOptions.config.flowTransRadiusFraction, 1.0e-3, 1.0e-15, "Config density-level fraction mismatch.");
    requireGeneratorValidationSuccess(configOnly.runOptions.config, "Config-only density-normal flow-trans setup should validate.");

    const ParsedRunOptions cliOverride = parseArguments(
        {configFile.path().string(), "--flow-trans-rho0", "0.75", "--flow-trans-profile-power", "1.8", "--flow-trans-radius", "density-percentile:0.95"});
    requireNear(cliOverride.runOptions.config.flowTransRho0, 0.75, 1.0e-12, "CLI should override config flow-trans-rho0.");
    requireNear(cliOverride.runOptions.config.flowTransProfilePower, 1.8, 1.0e-12, "CLI should override config flow-trans-profile-power.");
    require(cliOverride.runOptions.config.flowTransRadiusMode == blastwave::FlowTransRadiusMode::DensityPercentile,
            "CLI should override config flow-trans-radius mode.");
    requireNear(cliOverride.runOptions.config.flowTransRadiusFraction, 0.95, 1.0e-12, "CLI should override config flow-trans-radius fraction.");
  }

  // Verify the new radius-resolution preset parses from config and CLI and keeps validation happy.
  void runFlowTransRadiusResolutionParseTest() {
    const TemporaryConfigFile balancedConfig(
        "flow-velocity-sampler = density-normal\n"
        "flow-trans-radius-resolution = balanced\n");
    const ParsedRunOptions balanced = parseArguments({balancedConfig.path().string()});
    require(balanced.runOptions.config.flowTransRadiusResolution == blastwave::FlowTransRadiusResolution::Balanced,
            "Config flow-trans-radius-resolution should parse balanced.");
    require(balanced.runOptions.config.hasFlowTransRadiusResolution,
            "Config flow-trans-radius-resolution should mark the option explicit.");
    requireGeneratorValidationSuccess(balanced.runOptions.config, "Balanced flow-trans-radius-resolution should validate for density-normal.");

    const TemporaryConfigFile preciseConfig(
        "flow-velocity-sampler = density-normal\n"
        "flow-trans-radius-resolution = precise\n");
    const ParsedRunOptions precise = parseArguments({preciseConfig.path().string()});
    require(precise.runOptions.config.flowTransRadiusResolution == blastwave::FlowTransRadiusResolution::Precise,
            "Config flow-trans-radius-resolution should parse precise.");
    require(precise.runOptions.config.hasFlowTransRadiusResolution,
            "Config precise flow-trans-radius-resolution should mark the option explicit.");
    requireGeneratorValidationSuccess(precise.runOptions.config, "Precise flow-trans-radius-resolution should validate for density-normal.");

    const TemporaryConfigFile fastConfig(
        "flow-velocity-sampler = density-normal\n"
        "flow-trans-radius-resolution = fast\n");
    const ParsedRunOptions fast = parseArguments({fastConfig.path().string()});
    require(fast.runOptions.config.flowTransRadiusResolution == blastwave::FlowTransRadiusResolution::Fast,
            "Config flow-trans-radius-resolution should parse fast.");
    require(fast.runOptions.config.hasFlowTransRadiusResolution,
            "Config fast flow-trans-radius-resolution should mark the option explicit.");
    requireGeneratorValidationSuccess(fast.runOptions.config, "Fast flow-trans-radius-resolution should validate for density-normal.");

    const ParsedRunOptions cliBalanced =
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius-resolution", "balanced"});
    require(cliBalanced.runOptions.config.flowTransRadiusResolution == blastwave::FlowTransRadiusResolution::Balanced,
            "CLI flow-trans-radius-resolution should parse balanced.");
    require(cliBalanced.runOptions.config.hasFlowTransRadiusResolution,
            "CLI balanced flow-trans-radius-resolution should mark the option explicit.");
    requireGeneratorValidationSuccess(cliBalanced.runOptions.config, "CLI balanced flow-trans-radius-resolution should validate for density-normal.");

    const ParsedRunOptions cliPrecise =
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius-resolution", "precise"});
    require(cliPrecise.runOptions.config.flowTransRadiusResolution == blastwave::FlowTransRadiusResolution::Precise,
            "CLI flow-trans-radius-resolution should parse precise.");
    require(cliPrecise.runOptions.config.hasFlowTransRadiusResolution,
            "CLI precise flow-trans-radius-resolution should mark the option explicit.");
    requireGeneratorValidationSuccess(cliPrecise.runOptions.config, "CLI precise flow-trans-radius-resolution should validate for density-normal.");

    const ParsedRunOptions cliFast =
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius-resolution", "fast"});
    require(cliFast.runOptions.config.flowTransRadiusResolution == blastwave::FlowTransRadiusResolution::Fast,
            "CLI flow-trans-radius-resolution should parse fast.");
    require(cliFast.runOptions.config.hasFlowTransRadiusResolution,
            "CLI fast flow-trans-radius-resolution should mark the option explicit.");
    requireGeneratorValidationSuccess(cliFast.runOptions.config, "CLI fast flow-trans-radius-resolution should validate for density-normal.");
  }

  // Verify explicit CLI overrides win over config-file presets for the new resolution knob.
  void runFlowTransRadiusResolutionCliOverrideParseTest() {
    const TemporaryConfigFile configFile(
        "flow-velocity-sampler = density-normal\n"
        "flow-trans-radius-resolution = precise\n");
    const ParsedRunOptions parsed = parseArguments({configFile.path().string(), "--flow-trans-radius-resolution", "fast"});
    require(parsed.runOptions.config.flowTransRadiusResolution == blastwave::FlowTransRadiusResolution::Fast,
            "CLI flow-trans-radius-resolution should override the config-file preset.");
    require(parsed.runOptions.config.hasFlowTransRadiusResolution,
            "CLI override should still mark flow-trans-radius-resolution explicit.");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "CLI override flow-trans-radius-resolution should validate for density-normal.");
  }

  // Reject unsupported resolution labels and empty values before they can reach validation.
  void runInvalidFlowTransRadiusResolutionRejectsTest() {
    const std::vector<std::string> invalidValues = {"medium", "240x256", ""};
    for (const std::string &invalidValue : invalidValues) {
      requireParseFailureContains({"--flow-trans-radius-resolution", invalidValue},
                                  "flow-trans-radius-resolution",
                                  "Invalid flow-trans-radius-resolution should be rejected: " + invalidValue);
    }

    const TemporaryConfigFile emptyConfig(
        "flow-velocity-sampler = density-normal\n"
        "flow-trans-radius-resolution =\n");
    requireParseFailureContains({emptyConfig.path().string()},
                                "flow-trans-radius-resolution",
                                "Empty config flow-trans-radius-resolution should be rejected.");
  }

  // Explicitly configured resolution must stay density-normal-only without breaking the default covariance path.
  void runFlowTransRadiusResolutionValidationRejectsTest() {
    blastwave::BlastWaveConfig config;
    config.flowVelocitySamplerMode = blastwave::FlowVelocitySamplerMode::CovarianceEllipse;
    config.flowTransRadiusResolution = blastwave::FlowTransRadiusResolution::Balanced;
    config.hasFlowTransRadiusResolution = true;
    requireGeneratorValidationFailure(config, "Explicit flow-trans-radius-resolution should be rejected outside density-normal.");
  }

  // Exercise the new magnitude-mode parser against the default, config, CLI, and override paths.
  void runFlowTransMagnitudeModeParseTest() {
    const ParsedRunOptions defaultParsed = parseArguments({});
    requireFlowTransMagnitudeMode(defaultParsed.runOptions.config,
                                  blastwave::FlowTransMagnitudeMode::RadiusProfile,
                                  "Default flow-trans-magnitude-mode should stay radius-profile.");
    require(!defaultParsed.runOptions.config.hasFlowTransMagnitudeMode,
            "Default flow-trans-magnitude-mode should stay implicit.");

    const TemporaryConfigFile radiusProfileConfig("flow-trans-magnitude-mode = radius-profile\n");
    const ParsedRunOptions radiusProfileParsed = parseArguments({radiusProfileConfig.path().string()});
    requireFlowTransMagnitudeMode(radiusProfileParsed.runOptions.config,
                                  blastwave::FlowTransMagnitudeMode::RadiusProfile,
                                  "Config flow-trans-magnitude-mode should parse radius-profile.");
    requireHasFlowTransMagnitudeMode(radiusProfileParsed.runOptions.config,
                                     "Config radius-profile flow-trans-magnitude-mode should mark the option explicit.");

    const TemporaryConfigFile shellGradientConfig("flow-trans-magnitude-mode = shell-gradient-corrected\n");
    const ParsedRunOptions shellGradientParsed = parseArguments({shellGradientConfig.path().string()});
    requireFlowTransMagnitudeMode(shellGradientParsed.runOptions.config,
                                  blastwave::FlowTransMagnitudeMode::ShellGradientCorrected,
                                  "Config flow-trans-magnitude-mode should parse shell-gradient-corrected.");
    requireHasFlowTransMagnitudeMode(shellGradientParsed.runOptions.config,
                                     "Config shell-gradient-corrected flow-trans-magnitude-mode should mark the option explicit.");

    const ParsedRunOptions cliShellGradient = parseArguments({"--flow-trans-magnitude-mode", "shell-gradient-corrected"});
    requireFlowTransMagnitudeMode(cliShellGradient.runOptions.config,
                                  blastwave::FlowTransMagnitudeMode::ShellGradientCorrected,
                                  "CLI flow-trans-magnitude-mode should parse shell-gradient-corrected.");
    requireHasFlowTransMagnitudeMode(cliShellGradient.runOptions.config,
                                     "CLI shell-gradient-corrected flow-trans-magnitude-mode should mark the option explicit.");

    const ParsedRunOptions overridden = parseArguments({shellGradientConfig.path().string(), "--flow-trans-magnitude-mode", "radius-profile"});
    requireFlowTransMagnitudeMode(overridden.runOptions.config,
                                  blastwave::FlowTransMagnitudeMode::RadiusProfile,
                                  "CLI flow-trans-magnitude-mode should override the config-file shell-gradient-corrected value.");
    requireHasFlowTransMagnitudeMode(overridden.runOptions.config,
                                     "CLI override should still mark flow-trans-magnitude-mode explicit.");
  }

  // Reject bad magnitude-mode labels with a diagnostic that names the new contract values.
  void runInvalidFlowTransMagnitudeModeRejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--flow-trans-magnitude-mode", "not-a-mode"}));
    } catch (const std::invalid_argument &error) {
      threw = std::string(error.what()).find("flow-trans-magnitude-mode") != std::string::npos;
    }
    require(threw, "Invalid CLI flow-trans-magnitude-mode should be rejected.");

    const TemporaryConfigFile invalidConfig("flow-trans-magnitude-mode = not-a-mode\n");
    threw = false;
    try {
      static_cast<void>(parseArguments({invalidConfig.path().string()}));
    } catch (const std::invalid_argument &error) {
      threw = std::string(error.what()).find("flow-trans-magnitude-mode") != std::string::npos;
    }
    require(threw, "Invalid config flow-trans-magnitude-mode should be rejected.");
  }

  // Validate the density-defined shell-gradient path on the two supported radius selectors.
  void runShellGradientCorrectedDensityRadiusValidationTest() {
    const blastwave::BlastWaveConfig percentileConfig =
        makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityPercentile, 0.95, 0.25, 1.0e-4, 0.2);
    requireGeneratorValidationSuccess(percentileConfig, "shell-gradient-corrected with density-percentile should validate.");

    const blastwave::BlastWaveConfig levelConfig =
        makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityLevel, 1.0e-3, 0.25, 1.0e-4, 0.2);
    requireGeneratorValidationSuccess(levelConfig, "shell-gradient-corrected with density-level should validate.");
  }

  // Reject shell-gradient usage when the sampler or radius choice is incompatible with the contract.
  void runShellGradientCorrectedValidationRejectsTest() {
    const blastwave::BlastWaveConfig covarianceRadius =
        makeShellGradientConfig(blastwave::FlowTransRadiusMode::Covariance, 0.0, 0.25, 1.0e-4, 0.2);
    requireGeneratorValidationFailure(covarianceRadius, "shell-gradient-corrected with covariance radius should be rejected.");

    const blastwave::BlastWaveConfig nonDensityNormalSampler = makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityPercentile,
                                                                                       0.95,
                                                                                       0.25,
                                                                                       1.0e-4,
                                                                                       0.2,
                                                                                       blastwave::FlowVelocitySamplerMode::CovarianceEllipse);
    requireGeneratorValidationFailure(nonDensityNormalSampler, "shell-gradient-corrected should be rejected outside density-normal.");
  }

  // Reject explicit gradient knobs when the magnitude mode stays on the legacy radius-profile path.
  void runRadiusProfileGradientKnobValidationRejectsTest() {
    blastwave::BlastWaveConfig radiusProfileGradientStrength;
    radiusProfileGradientStrength.flowVelocitySamplerMode = blastwave::FlowVelocitySamplerMode::DensityNormal;
    radiusProfileGradientStrength.densityEvolutionMode = blastwave::DensityEvolutionMode::AffineGaussianResponse;
    radiusProfileGradientStrength.flowTransRadiusMode = blastwave::FlowTransRadiusMode::DensityPercentile;
    radiusProfileGradientStrength.flowTransRadiusFraction = 0.95;
    radiusProfileGradientStrength.hasFlowTransRadius = true;
    radiusProfileGradientStrength.flowTransMagnitudeMode = blastwave::FlowTransMagnitudeMode::RadiusProfile;
    radiusProfileGradientStrength.hasFlowTransMagnitudeMode = true;
    radiusProfileGradientStrength.flowTransGradientStrength = 0.25;
    radiusProfileGradientStrength.hasFlowTransGradientStrength = true;
    requireGeneratorValidationFailure(radiusProfileGradientStrength,
                                      "Explicit flow-trans-gradient-strength should be rejected unless shell-gradient-corrected is selected.");
  }

  // Keep the new floor and cap bounds finite and within the allowed multiplicative range.
  void runShellGradientBoundValidationRejectsTest() {
    const blastwave::BlastWaveConfig zeroFloor = makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityPercentile, 0.95, 0.25, 0.0, 0.2);
    requireGeneratorValidationFailure(zeroFloor, "Zero flow-trans-gradient-density-floor-fraction should fail validation.");

    const blastwave::BlastWaveConfig negativeFloor =
        makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityPercentile, 0.95, 0.25, -1.0e-4, 0.2);
    requireGeneratorValidationFailure(negativeFloor, "Negative flow-trans-gradient-density-floor-fraction should fail validation.");

    const blastwave::BlastWaveConfig negativeCap = makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityPercentile, 0.95, 0.25, 1.0e-4, -0.1);
    requireGeneratorValidationFailure(negativeCap, "Negative flow-trans-gradient-max-factor-delta should fail validation.");

    const blastwave::BlastWaveConfig largeCap = makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityPercentile, 0.95, 0.25, 1.0e-4, 1.0);
    requireGeneratorValidationFailure(largeCap, "flow-trans-gradient-max-factor-delta >= 1 should fail validation.");
  }

  void runDeprecatedTransverseFlowNamesRejectTest() {
    requireParseFailureContains({"--rho0", "0.9"}, "flow-trans-rho0", "Deprecated CLI rho0 should point to flow-trans-rho0.");
    requireParseFailureContains({"--flow-power", "1.2"}, "flow-trans-profile-power", "Deprecated CLI flow-power should point to flow-trans-profile-power.");

    const TemporaryConfigFile rho0Config("rho0 = 0.9\n");
    requireParseFailureContains({rho0Config.path().string()}, "flow-trans-rho0", "Deprecated config rho0 should point to flow-trans-rho0.");

    const TemporaryConfigFile flowPowerConfig("flow-power = 1.2\n");
    requireParseFailureContains(
        {flowPowerConfig.path().string()}, "flow-trans-profile-power", "Deprecated config flow-power should point to flow-trans-profile-power.");
  }

  void runFlowTransRadiusParseVariantsTest() {
    const ParsedRunOptions covariance =
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius", "covariance"});
    require(covariance.runOptions.config.flowTransRadiusMode == blastwave::FlowTransRadiusMode::Covariance,
            "flow-trans-radius=covariance parse mismatch.");

    const ParsedRunOptions percentile =
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius", "density-percentile:0.95"});
    require(percentile.runOptions.config.flowTransRadiusMode == blastwave::FlowTransRadiusMode::DensityPercentile,
            "flow-trans-radius density-percentile parse mismatch.");
    requireNear(percentile.runOptions.config.flowTransRadiusFraction, 0.95, 1.0e-12, "density-percentile fraction mismatch.");

    const ParsedRunOptions level =
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius", "density-level:1.0e-3"});
    require(level.runOptions.config.flowTransRadiusMode == blastwave::FlowTransRadiusMode::DensityLevel,
            "flow-trans-radius density-level parse mismatch.");
    requireNear(level.runOptions.config.flowTransRadiusFraction, 1.0e-3, 1.0e-15, "density-level fraction mismatch.");
  }

  void runInvalidFlowTransRadiusRejectsTest() {
    const std::vector<std::string> invalidValues = {
        "density-percentile",
        "density-percentile:0",
        "density-percentile:1",
        "density-percentile:-0.1",
        "density-percentile:abc",
        "density-level:0",
        "density-level:1",
        "density-level:1.2",
        "density-level:-1.0e-3",
        "density-level:abc",
        "not-a-mode",
    };
    for (const std::string &invalidValue : invalidValues) {
      requireParseFailureContains({"--flow-trans-radius", invalidValue}, "flow-trans-radius", "Invalid flow-trans-radius should be rejected: " + invalidValue);
    }
    requireParseFailureContains({"--flow-trans-radius", "density-level:1.0"},
                                "0 < fraction < 1",
                                "density-level=1.0 should be rejected with bounded-fraction text.");
  }

  void runDensityLevelFlowTransValidationRejectsTest() {
    const blastwave::BlastWaveConfig densityLevelOne = makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityLevel, 1.0, 0.25, 1.0e-4, 0.2);
    const blastwave::BlastWaveConfig densityLevelTooLarge =
        makeShellGradientConfig(blastwave::FlowTransRadiusMode::DensityLevel, 1.2, 0.25, 1.0e-4, 0.2);
    requireGeneratorValidationFailureContains(
        densityLevelOne, "0 < fraction < 1", "Generator validation should reject density-level:1.0 with bounded-fraction text.");
    requireGeneratorValidationFailureContains(
        densityLevelTooLarge, "0 < fraction < 1", "Generator validation should reject density-level:1.2 with bounded-fraction text.");
  }

  void runFlowTransValidationRejectsTest() {
    const ParsedRunOptions negativeDirection =
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-direction-gradient-fraction", "-0.1"});
    requireGeneratorValidationFailure(negativeDirection.runOptions.config, "Negative flow-trans direction fraction should fail validation.");

    const ParsedRunOptions largeDirection =
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-direction-gradient-fraction", "1.1"});
    requireGeneratorValidationFailure(largeDirection.runOptions.config, "Large flow-trans direction fraction should fail validation.");

    const ParsedRunOptions covarianceWithDirection =
        parseArguments({"--flow-velocity-sampler", "covariance-ellipse", "--flow-trans-direction-gradient-fraction", "0.5"});
    requireGeneratorValidationFailure(covarianceWithDirection.runOptions.config, "flow-trans direction should be rejected outside density-normal.");

    const ParsedRunOptions affineWithRadius =
        parseArguments({"--flow-velocity-sampler", "affine-effective", "--density-evolution", "affine-gaussian", "--flow-trans-radius", "covariance"});
    requireGeneratorValidationFailure(affineWithRadius.runOptions.config, "flow-trans radius should be rejected outside density-normal.");

    const ParsedRunOptions gradientWithRadius = parseArguments(
        {"--flow-velocity-sampler", "gradient-response", "--density-evolution", "gradient-response", "--flow-trans-radius", "density-percentile:0.95"});
    requireGeneratorValidationFailure(gradientWithRadius.runOptions.config, "flow-trans radius should be rejected for gradient-response.");
  }

  void runDensityNormalFlowTransValidationSuccessTest() {
    requireGeneratorValidationSuccess(parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius", "covariance"}).runOptions.config,
                                      "Density-normal covariance flow-trans radius should validate.");
    requireGeneratorValidationSuccess(
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius", "density-percentile:0.95"}).runOptions.config,
        "Density-normal percentile flow-trans radius should validate.");
    requireGeneratorValidationSuccess(
        parseArguments({"--flow-velocity-sampler", "density-normal", "--flow-trans-radius", "density-level:1.0e-3"}).runOptions.config,
        "Density-normal density-level flow-trans radius should validate.");
  }

  void runCliSamplerAndDensityEvolutionParseTest() {
    const ParsedRunOptions parsed = parseArguments(
        {"--flow-velocity-sampler",
         "density-normal",
         "--density-evolution",
         "affine-gaussian",
         "--density-normal-kappa-compensation",
         "--kappa2",
         "0.25",
         "--flow-density-sigma",
         "0.8",
         "--affine-lambda-in",
         "1.14",
         "--affine-lambda-out",
         "1.01",
         "--affine-sigma-evo",
         "0.35"});
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::DensityNormal,
            "CLI sampler parse should accept density-normal.");
    require(parsed.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse,
            "CLI density-evolution parse should accept affine-gaussian.");
    require(parsed.runOptions.config.densityNormalKappaCompensation, "CLI density-normal kappa compensation flag should enable the opt-in modulation.");
    requireNear(parsed.runOptions.config.kappa2, 0.25, 1.0e-12, "CLI kappa2 parse mismatch.");
    requireNear(parsed.runOptions.config.flowDensitySigma, 0.8, 1.0e-12, "CLI flow density sigma parse mismatch.");
    requireNear(parsed.runOptions.config.affineLambdaIn, 1.14, 1.0e-12, "CLI affine lambda in parse mismatch.");
    requireNear(parsed.runOptions.config.affineLambdaOut, 1.01, 1.0e-12, "CLI affine lambda out parse mismatch.");
    requireNear(parsed.runOptions.config.affineSigmaEvo, 0.35, 1.0e-12, "CLI affine sigma evo parse mismatch.");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "Affine density-normal kappa compensation should be accepted by generator validation.");
  }

  void runConfigAndOverrideParseTest() {
    const TemporaryConfigFile configFile(
        "flow-velocity-sampler = density-normal\n"
        "density-evolution = affine-gaussian\n"
        "kappa2 = 0.3\n"
        "density-normal-kappa-compensation = true\n"
        "flow-density-sigma = 0.7\n"
        "affine-lambda-in = 1.22\n"
        "affine-lambda-out = 1.08\n"
        "affine-sigma-evo = 0.45\n"
        "output = qa/test.root\n");
    const ParsedRunOptions parsed = parseArguments(
        {configFile.path().string(),
         "--flow-velocity-sampler",
         "covariance-ellipse",
         "--density-evolution",
         "affine-gaussian",
         "--no-density-normal-kappa-compensation",
         "--kappa2",
         "0.45",
         "--affine-lambda-out",
         "1.02"});
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
            "CLI override should win over config-file sampler value.");
    require(parsed.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse,
            "CLI override should win over config-file density-evolution value.");
    requireNear(parsed.runOptions.config.kappa2, 0.45, 1.0e-12, "CLI override should win over config-file kappa2 value.");
    require(!parsed.runOptions.config.densityNormalKappaCompensation, "CLI --no-density-normal-kappa-compensation should disable the config-file opt-in.");
    requireNear(parsed.runOptions.config.flowDensitySigma, 0.7, 1.0e-12, "Config-file flow density sigma parse mismatch.");
    requireNear(parsed.runOptions.config.affineLambdaIn, 1.22, 1.0e-12, "Config-file affine lambda in parse mismatch.");
    requireNear(parsed.runOptions.config.affineLambdaOut, 1.02, 1.0e-12, "CLI override should win over config-file affine lambda out value.");
    requireNear(parsed.runOptions.config.affineSigmaEvo, 0.45, 1.0e-12, "Config-file affine sigma evo parse mismatch.");
    require(parsed.runOptions.outputPath == (configFile.path().parent_path() / "qa/test.root").lexically_normal().string(),
            "Relative config-file output path should resolve against the config directory.");
  }

  void runGradientConfigParseTest() {
    const TemporaryConfigFile configFile(
        "density-evolution = gradient-response\n"
        "flow-velocity-sampler = gradient-response\n"
        "gradient-sigma-em = 0.25\n"
        "gradient-sigma-dyn = 1.35\n"
        "gradient-density-floor-fraction = 0.0002\n"
        "gradient-density-cutoff-fraction = 7.0e-7\n"
        "gradient-displacement-max = 1.8\n"
        "gradient-displacement-kappa = 0.9\n"
        "gradient-diffusion-sigma = 0.05\n"
        "gradient-vmax = 0.6\n"
        "gradient-velocity-kappa = 1.4\n"
        "debug-gradient-response = true\n"
        "cooper-frye-weight = mt-cosh\n");
    const ParsedRunOptions parsed = parseArguments({configFile.path().string()});

    require(parsed.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::GradientResponse,
            "Config-file density-evolution should accept gradient-response.");
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::GradientResponse,
            "Config-file flow-velocity-sampler should accept gradient-response.");
    requireNear(parsed.runOptions.config.gradientSigmaEm, 0.25, 1.0e-12, "Config-file gradient-sigma-em parse mismatch.");
    requireNear(parsed.runOptions.config.gradientSigmaDyn, 1.35, 1.0e-12, "Config-file gradient-sigma-dyn parse mismatch.");
    requireNear(parsed.runOptions.config.gradientDensityFloorFraction, 0.0002, 1.0e-16, "Config-file gradient density floor fraction parse mismatch.");
    requireNear(parsed.runOptions.config.gradientDensityCutoffFraction, 7.0e-7, 1.0e-18, "Config-file gradient density cutoff fraction parse mismatch.");
    requireNear(parsed.runOptions.config.gradientDisplacementMax, 1.8, 1.0e-12, "Config-file gradient displacement max parse mismatch.");
    requireNear(parsed.runOptions.config.gradientDisplacementKappa, 0.9, 1.0e-12, "Config-file gradient displacement kappa parse mismatch.");
    requireNear(parsed.runOptions.config.gradientDiffusionSigma, 0.05, 1.0e-12, "Config-file gradient diffusion sigma parse mismatch.");
    requireNear(parsed.runOptions.config.gradientVMax, 0.6, 1.0e-12, "Config-file gradient vmax parse mismatch.");
    requireNear(parsed.runOptions.config.gradientVelocityKappa, 1.4, 1.0e-12, "Config-file gradient velocity kappa parse mismatch.");
    require(parsed.runOptions.config.debugGradientResponse, "Config-file debug-gradient-response should parse to true.");
    require(parsed.runOptions.config.cooperFryeWeightMode == blastwave::CooperFryeWeightMode::MtCosh,
            "Config-file cooper-frye-weight should accept mt-cosh.");
  }

  void runAffineEffectiveParseTest() {
    const ParsedRunOptions parsed = parseArguments({"--flow-velocity-sampler",
                                                    "affine-effective",
                                                    "--density-evolution",
                                                    "affine-gaussian",
                                                    "--affine-delta-tau-ref",
                                                    "8.5",
                                                    "--affine-kappa-flow",
                                                    "9.0",
                                                    "--affine-kappa-aniso",
                                                    "1.7",
                                                    "--affine-u-max",
                                                    "0.72",
                                                    "--affine-effective-mode",
                                                    "full-tensor"});
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::AffineEffective,
            "CLI should accept affine-effective as a flow-velocity sampler.");
    requireNear(parsed.runOptions.config.affineDeltaTauRef, 8.5, 1.0e-12, "CLI affine-delta-tau-ref parse mismatch.");
    requireNear(parsed.runOptions.config.affineKappaFlow, 9.0, 1.0e-12, "CLI affine-kappa-flow parse mismatch.");
    requireNear(parsed.runOptions.config.affineKappaAniso, 1.7, 1.0e-12, "CLI affine-kappa-aniso parse mismatch.");
    requireNear(parsed.runOptions.config.affineUMax, 0.72, 1.0e-12, "CLI affine-u-max parse mismatch.");
    require(parsed.runOptions.config.affineEffectiveMode == blastwave::AffineEffectiveMode::FullTensor,
            "CLI affine-effective-mode parse mismatch.");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "Affine-effective affine-gaussian combination should pass generator validation.");
  }

  void runAffineEffectiveModeConfigAndOverrideParseTest() {
    const TemporaryConfigFile configFile(
        "flow-velocity-sampler = affine-effective\n"
        "density-evolution = affine-gaussian\n"
        "affine-effective-mode = full-tensor\n"
        "affine-kappa-aniso = 9.0\n");
    const ParsedRunOptions configOnly = parseArguments({configFile.path().string()});
    require(configOnly.runOptions.config.affineEffectiveMode == blastwave::AffineEffectiveMode::FullTensor,
            "Config affine-effective-mode should parse full-tensor.");
    requireNear(configOnly.runOptions.config.affineKappaAniso, 9.0, 1.0e-12, "Config affine-kappa-aniso parse mismatch.");
    requireGeneratorValidationSuccess(configOnly.runOptions.config, "Legacy affine-kappa-aniso should remain validation-compatible.");

    const ParsedRunOptions cliOverride = parseArguments({configFile.path().string(), "--affine-effective-mode", "additive-rho"});
    require(cliOverride.runOptions.config.affineEffectiveMode == blastwave::AffineEffectiveMode::AdditiveRho,
            "CLI affine-effective-mode should override config value.");
  }

  void runGradientCliOverrideParseTest() {
    const TemporaryConfigFile configFile(
        "density-evolution = gradient-response\n"
        "flow-velocity-sampler = gradient-response\n"
        "gradient-sigma-em = 0.10\n"
        "gradient-sigma-dyn = 1.20\n"
        "gradient-density-floor-fraction = 0.0004\n"
        "gradient-density-cutoff-fraction = 2.0e-6\n"
        "gradient-displacement-max = 1.0\n"
        "gradient-displacement-kappa = 0.7\n"
        "gradient-diffusion-sigma = 0.20\n"
        "gradient-vmax = 0.3\n"
        "gradient-velocity-kappa = 0.8\n"
        "debug-gradient-response = false\n"
        "cooper-frye-weight = mt-cosh\n");
    const ParsedRunOptions parsed = parseArguments({configFile.path().string(),
                                                    "--gradient-sigma-em",
                                                    "0.35",
                                                    "--gradient-sigma-dyn",
                                                    "1.50",
                                                    "--gradient-density-floor-fraction",
                                                    "0.0006",
                                                    "--gradient-density-cutoff-fraction",
                                                    "3.0e-6",
                                                    "--gradient-displacement-max",
                                                    "2.25",
                                                    "--gradient-displacement-kappa",
                                                    "1.2",
                                                    "--gradient-diffusion-sigma",
                                                    "0.0",
                                                    "--gradient-vmax",
                                                    "0.55",
                                                    "--gradient-velocity-kappa",
                                                    "1.6",
                                                    "--debug-gradient-response",
                                                    "--cooper-frye-weight",
                                                    "none"});

    requireNear(parsed.runOptions.config.gradientSigmaEm, 0.35, 1.0e-12, "CLI should override gradient-sigma-em.");
    requireNear(parsed.runOptions.config.gradientSigmaDyn, 1.50, 1.0e-12, "CLI should override gradient-sigma-dyn.");
    requireNear(parsed.runOptions.config.gradientDensityFloorFraction, 0.0006, 1.0e-16, "CLI should override gradient density floor fraction.");
    requireNear(parsed.runOptions.config.gradientDensityCutoffFraction, 3.0e-6, 1.0e-18, "CLI should override gradient density cutoff fraction.");
    requireNear(parsed.runOptions.config.gradientDisplacementMax, 2.25, 1.0e-12, "CLI should override gradient displacement max.");
    requireNear(parsed.runOptions.config.gradientDisplacementKappa, 1.2, 1.0e-12, "CLI should override gradient displacement kappa.");
    requireNear(parsed.runOptions.config.gradientDiffusionSigma, 0.0, 1.0e-12, "CLI should override gradient diffusion sigma.");
    requireNear(parsed.runOptions.config.gradientVMax, 0.55, 1.0e-12, "CLI should override gradient vmax.");
    requireNear(parsed.runOptions.config.gradientVelocityKappa, 1.6, 1.0e-12, "CLI should override gradient velocity kappa.");
    require(parsed.runOptions.config.debugGradientResponse, "CLI should enable debug-gradient-response.");
    require(parsed.runOptions.config.cooperFryeWeightMode == blastwave::CooperFryeWeightMode::None,
            "CLI should override cooper-frye-weight back to none.");
  }

  void runInvalidSamplerRejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--flow-velocity-sampler", "not-a-sampler"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "Invalid flow velocity sampler should be rejected.");
  }

  void runInvalidDensityEvolutionRejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--density-evolution", "not-a-mode"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "Invalid density-evolution mode should be rejected.");
  }

  void runInvalidAffineEffectiveModeRejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--affine-effective-mode", "invalid-mode"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "Invalid affine-effective-mode should be rejected.");
  }

  void runInvalidCooperFryeWeightRejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--cooper-frye-weight", "not-a-weight"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "Invalid cooper-frye-weight should be rejected.");
  }

  void runInitialGeometryDefaultParseTest() {
    const ParsedRunOptions parsed = parseArguments({});
    require(parsed.runOptions.config.initialGeometryMode == blastwave::InitialGeometryMode::Glauber,
            "Default initial-geometry should remain Glauber.");
    require(!parsed.runOptions.config.initialGeometryFluctuate, "Default initial-geometry-fluctuate should be false.");
    require(!parsed.runOptions.config.hasInitialGeometrySourceCountMin && !parsed.runOptions.config.hasInitialGeometrySourceCountMax,
            "Default source-count fluctuation range should stay implicit.");
    require(!parsed.runOptions.config.hasInitialGeometryA2Min && !parsed.runOptions.config.hasInitialGeometryA2Max,
            "Default a2 fluctuation range should stay implicit.");
    require(!parsed.runOptions.config.hasInitialGeometryA3Min && !parsed.runOptions.config.hasInitialGeometryA3Max,
            "Default a3 fluctuation range should stay implicit.");
    require(!parsed.runOptions.config.hasInitialGeometryR2xMin && !parsed.runOptions.config.hasInitialGeometryR2xMax,
            "Default r2x fluctuation range should stay implicit.");
    require(!parsed.runOptions.config.hasInitialGeometryR2yMin && !parsed.runOptions.config.hasInitialGeometryR2yMax,
            "Default r2y fluctuation range should stay implicit.");
    require(!parsed.runOptions.config.hasInitialGeometryR3Min && !parsed.runOptions.config.hasInitialGeometryR3Max,
            "Default r3 fluctuation range should stay implicit.");
    require(!parsed.runOptions.config.hasInitialGeometrySigma3Min && !parsed.runOptions.config.hasInitialGeometrySigma3Max,
            "Default sigma3 fluctuation range should stay implicit.");
    require(parsed.runOptions.config.initialGeometrySourceCount == 600, "Default initial-geometry-source-count should be 600.");
    requireNear(parsed.runOptions.config.initialGeometryR0, 1.2, 1.0e-12, "Default initial-geometry-r0 mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryA2, 0.0, 1.0e-12, "Default initial-geometry-a2 should be zero.");
    requireNear(parsed.runOptions.config.initialGeometryR2x, 1.8, 1.0e-12, "Default initial-geometry-r2x mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryR2y, 1.8, 1.0e-12, "Default initial-geometry-r2y mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryR3, 1.8, 1.0e-12, "Default initial-geometry-r3 mismatch.");
    requireNear(parsed.runOptions.config.initialGeometrySigma3, 0.6, 1.0e-12, "Default initial-geometry-sigma3 mismatch.");
    require(!parsed.runOptions.config.debugInitialGeometry, "Default debug-initial-geometry should be disabled.");
  }

  void runInitialGeometryParseAndOverrideTest() {
    const ParsedRunOptions parsed = parseArguments({"--initial-geometry",
                                                   "response-test-023",
                                                   "--initial-geometry-source-count",
                                                   "420",
                                                   "--initial-geometry-r0",
                                                   "1.5",
                                                   "--initial-geometry-a2",
                                                   "0.2",
                                                   "--initial-geometry-r2x",
                                                   "2.0",
                                                   "--initial-geometry-r2y",
                                                   "1.6",
                                                   "--initial-geometry-phi2",
                                                   "0.42",
                                                   "--initial-geometry-a3",
                                                   "0.15",
                                                   "--initial-geometry-r3",
                                                   "2.2",
                                                   "--initial-geometry-sigma3",
                                                   "0.35",
                                                   "--initial-geometry-phi3",
                                                   "1.1",
                                                   "--debug-initial-geometry"});
    require(parsed.runOptions.config.initialGeometryMode == blastwave::InitialGeometryMode::ResponseTest023,
            "CLI initial-geometry should parse as response-test-023.");
    require(parsed.runOptions.config.initialGeometrySourceCount == 420, "CLI source-count parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryR0, 1.5, 1.0e-12, "CLI initial-geometry-r0 parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryA2, 0.2, 1.0e-12, "CLI initial-geometry-a2 parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryR2x, 2.0, 1.0e-12, "CLI initial-geometry-r2x parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryR2y, 1.6, 1.0e-12, "CLI initial-geometry-r2y parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryPhi2, 0.42, 1.0e-12, "CLI initial-geometry-phi2 parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryA3, 0.15, 1.0e-12, "CLI initial-geometry-a3 parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryR3, 2.2, 1.0e-12, "CLI initial-geometry-r3 parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometrySigma3, 0.35, 1.0e-12, "CLI initial-geometry-sigma3 parse mismatch.");
    requireNear(parsed.runOptions.config.initialGeometryPhi3, 1.1, 1.0e-12, "CLI initial-geometry-phi3 parse mismatch.");
    require(parsed.runOptions.config.debugInitialGeometry, "CLI debug-initial-geometry should be enabled.");
  }

  // Verify CLI parsing for every response-test fluctuation range key.
  void runInitialGeometryFluctuationCliParseTest() {
    const ParsedRunOptions parsed = parseArguments({"--initial-geometry",
                                                   "response-test-023",
                                                   "--initial-geometry-fluctuate",
                                                   "--initial-geometry-source-count-min",
                                                   "180",
                                                   "--initial-geometry-source-count-max",
                                                   "700",
                                                   "--initial-geometry-a2-min",
                                                   "0.0",
                                                   "--initial-geometry-a2-max",
                                                   "1.0",
                                                   "--initial-geometry-a3-min",
                                                   "0.0",
                                                   "--initial-geometry-a3-max",
                                                   "0.7",
                                                   "--initial-geometry-r2x-min",
                                                   "1.8",
                                                   "--initial-geometry-r2x-max",
                                                   "3.2",
                                                   "--initial-geometry-r2y-min",
                                                   "0.9",
                                                   "--initial-geometry-r2y-max",
                                                   "1.8",
                                                   "--initial-geometry-r3-min",
                                                   "1.4",
                                                   "--initial-geometry-r3-max",
                                                   "3.0",
                                                   "--initial-geometry-sigma3-min",
                                                   "0.45",
                                                   "--initial-geometry-sigma3-max",
                                                   "1.3"});
    require(parsed.runOptions.config.initialGeometryMode == blastwave::InitialGeometryMode::ResponseTest023,
            "CLI fluctuation parse should keep response-test-023 mode.");
    requireFluctuatingRangeValues(parsed.runOptions.config, "CLI fluctuation parse");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "Complete CLI fluctuation config should pass validation.");
  }

  // Verify config-file parsing for every response-test fluctuation range key.
  void runInitialGeometryFluctuationConfigParseTest() {
    const TemporaryConfigFile configFile(
        "initial-geometry = response-test-023\n"
        "initial-geometry-fluctuate = true\n"
        "initial-geometry-source-count-min = 180\n"
        "initial-geometry-source-count-max = 700\n"
        "initial-geometry-a2-min = 0.0\n"
        "initial-geometry-a2-max = 1.0\n"
        "initial-geometry-a3-min = 0.0e0\n"
        "initial-geometry-a3-max = 7.0e-1\n"
        "initial-geometry-r2x-min = 1.8\n"
        "initial-geometry-r2x-max = 3.2\n"
        "initial-geometry-r2y-min = 0.9\n"
        "initial-geometry-r2y-max = 1.8\n"
        "initial-geometry-r3-min = 1.4\n"
        "initial-geometry-r3-max = 3.0\n"
        "initial-geometry-sigma3-min = 0.45\n"
        "initial-geometry-sigma3-max = 1.3\n");
    const ParsedRunOptions parsed = parseArguments({configFile.path().string()});
    requireFluctuatingRangeValues(parsed.runOptions.config, "Config fluctuation parse");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "Complete config-file fluctuation config should pass validation.");
  }

  // Verify CLI values override config-file fluctuation settings.
  void runInitialGeometryFluctuationCliOverrideTest() {
    const TemporaryConfigFile configFile(
        "initial-geometry = response-test-023\n"
        "initial-geometry-fluctuate = true\n"
        "initial-geometry-source-count-min = 20\n"
        "initial-geometry-source-count-max = 30\n"
        "initial-geometry-a2-min = 0.2\n"
        "initial-geometry-a2-max = 0.3\n"
        "initial-geometry-a3-min = 0.2\n"
        "initial-geometry-a3-max = 0.3\n"
        "initial-geometry-r2x-min = 2.0\n"
        "initial-geometry-r2x-max = 2.1\n"
        "initial-geometry-r2y-min = 1.0\n"
        "initial-geometry-r2y-max = 1.1\n"
        "initial-geometry-r3-min = 1.8\n"
        "initial-geometry-r3-max = 1.9\n"
        "initial-geometry-sigma3-min = 0.5\n"
        "initial-geometry-sigma3-max = 0.6\n");
    const ParsedRunOptions parsed =
        parseArguments({configFile.path().string(),
                        "--no-initial-geometry-fluctuate",
                        "--initial-geometry-source-count-min",
                        "180",
                        "--initial-geometry-source-count-max",
                        "700",
                        "--initial-geometry-a2-min",
                        "0.0",
                        "--initial-geometry-a2-max",
                        "1.0"});
    require(!parsed.runOptions.config.initialGeometryFluctuate, "CLI --no-initial-geometry-fluctuate should override config true.");
    require(parsed.runOptions.config.initialGeometrySourceCountMin == 180 && parsed.runOptions.config.initialGeometrySourceCountMax == 700,
            "CLI source-count range should override config.");
    requireNear(parsed.runOptions.config.initialGeometryA2Min, 0.0, 1.0e-12, "CLI a2-min should override config.");
    requireNear(parsed.runOptions.config.initialGeometryA2Max, 1.0, 1.0e-12, "CLI a2-max should override config.");
    requireNear(parsed.runOptions.config.initialGeometryA3Min, 0.2, 1.0e-12, "Unspecified a3-min should stay from config.");
    requireNear(parsed.runOptions.config.initialGeometryR2xMax, 2.1, 1.0e-12, "Unspecified r2x-max should stay from config.");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "Disabled fluctuation config should ignore configured ranges and validate.");
  }

  void runInitialGeometryConfigAndOverrideTest() {
    const TemporaryConfigFile configFile(
        "initial-geometry = response-test-023\n"
        "initial-geometry-source-count = 900\n"
        "initial-geometry-a2 = 0.4\n"
        "initial-geometry-r0 = 1.9\n"
        "initial-geometry-r2x = 2.3\n"
        "initial-geometry-r2y = 1.7\n"
        "initial-geometry-phi2 = 0.5\n"
        "initial-geometry-a3 = 0.21\n"
        "initial-geometry-r3 = 2.4\n"
        "initial-geometry-sigma3 = 0.7\n"
        "initial-geometry-phi3 = 0.9\n"
        "debug-initial-geometry = true\n");
    const ParsedRunOptions parsed = parseArguments(
        {configFile.path().string(), "--initial-geometry", "glauber", "--no-debug-initial-geometry", "--initial-geometry-source-count", "333"});
    require(parsed.runOptions.config.initialGeometryMode == blastwave::InitialGeometryMode::Glauber, "CLI initial-geometry should override config.");
    require(!parsed.runOptions.config.debugInitialGeometry, "CLI --no-debug-initial-geometry should disable config value.");
    require(parsed.runOptions.config.initialGeometrySourceCount == 333, "CLI source-count should override config.");
    requireNear(parsed.runOptions.config.initialGeometryR0, 1.9, 1.0e-12, "Config-file initial-geometry-r0 should remain when CLI does not override.");
  }

  void runInvalidInitialGeometryRejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--initial-geometry", "invalid-mode"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "Invalid initial-geometry mode should be rejected.");

    const ParsedRunOptions negativeSourceCount = parseArguments({"--initial-geometry", "response-test-023", "--initial-geometry-source-count", "-1"});
    requireGeneratorValidationFailure(negativeSourceCount.runOptions.config, "Negative initial-geometry-source-count should fail generator validation.");

    const ParsedRunOptions zeroR0 = parseArguments({"--initial-geometry", "response-test-023", "--initial-geometry-source-count", "10", "--initial-geometry-r0", "0.0"});
    requireGeneratorValidationFailure(zeroR0.runOptions.config, "Initial-geometry-r0 must be positive.");
  }

  // Reject incomplete, reversed, negative, or wrong-mode fluctuation contracts.
  void runInitialGeometryFluctuationValidationRejectsTest() {
    blastwave::BlastWaveConfig missingA2Max = makeValidFluctuating023Config();
    missingA2Max.hasInitialGeometryA2Max = false;
    requireGeneratorValidationFailureContains(missingA2Max, "a2-min/max", "Missing a2 max range should fail validation.");

    blastwave::BlastWaveConfig missingSourceMin = makeValidFluctuating023Config();
    missingSourceMin.hasInitialGeometrySourceCountMin = false;
    requireGeneratorValidationFailureContains(missingSourceMin, "source-count-min/max", "Missing source-count min range should fail validation.");

    blastwave::BlastWaveConfig reversedSourceCount = makeValidFluctuating023Config();
    reversedSourceCount.initialGeometrySourceCountMin = 18;
    reversedSourceCount.initialGeometrySourceCountMax = 17;
    requireGeneratorValidationFailureContains(reversedSourceCount, "source-count", "Reversed source-count range should fail validation.");

    blastwave::BlastWaveConfig reversedA2 = makeValidFluctuating023Config();
    reversedA2.initialGeometryA2Min = 0.9;
    reversedA2.initialGeometryA2Max = 0.1;
    requireGeneratorValidationFailureContains(reversedA2, "initial-geometry-a2", "Reversed a2 range should fail validation.");

    blastwave::BlastWaveConfig reversedR2x = makeValidFluctuating023Config();
    reversedR2x.initialGeometryR2xMin = 3.3;
    reversedR2x.initialGeometryR2xMax = 3.2;
    requireGeneratorValidationFailureContains(reversedR2x, "initial-geometry-r2x", "Reversed r2x range should fail validation.");

    blastwave::BlastWaveConfig negativeSourceCount = makeValidFluctuating023Config();
    negativeSourceCount.initialGeometrySourceCountMin = -1;
    requireGeneratorValidationFailureContains(negativeSourceCount, "source-count", "Negative source-count min should fail validation.");

    blastwave::BlastWaveConfig negativeA3 = makeValidFluctuating023Config();
    negativeA3.initialGeometryA3Min = -0.1;
    requireGeneratorValidationFailureContains(negativeA3, "initial-geometry-a3", "Negative a3 min should fail validation.");

    blastwave::BlastWaveConfig zeroSigma3 = makeValidFluctuating023Config();
    zeroSigma3.initialGeometrySigma3Min = 0.0;
    requireGeneratorValidationFailureContains(zeroSigma3, "initial-geometry-sigma3", "Zero sigma3 min should fail validation.");

    blastwave::BlastWaveConfig glauberFluctuate = makeValidFluctuating023Config();
    glauberFluctuate.initialGeometryMode = blastwave::InitialGeometryMode::Glauber;
    requireGeneratorValidationFailureContains(glauberFluctuate,
                                              "initial-geometry=response-test-023",
                                              "Glauber with initial-geometry-fluctuate should fail validation.");
  }

  void runDensityNormalKappaCompensationConfigParseTest() {
    const TemporaryConfigFile configFile(
        "flow-velocity-sampler = density-normal\n"
        "density-evolution = affine-gaussian\n"
        "density-normal-kappa-compensation = true\n"
        "flow-density-sigma = 0.6\n");
    const ParsedRunOptions parsed = parseArguments({configFile.path().string()});
    require(parsed.runOptions.config.densityNormalKappaCompensation, "Config-file density-normal-kappa-compensation should parse to true.");
    requireGeneratorValidationSuccess(parsed.runOptions.config, "Config-file density-normal kappa compensation should pass generator validation.");
  }

  void runDensityFlowMismatchRejectsTest() {
    const ParsedRunOptions densityOnly = parseArguments({"--density-evolution", "gradient-response"});
    require(densityOnly.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::GradientResponse,
            "Gradient-response density-evolution should parse before generator validation.");
    requireGeneratorValidationFailure(densityOnly.runOptions.config, "Gradient-response density without gradient flow should be rejected by the generator.");

    const ParsedRunOptions flowOnly = parseArguments({"--flow-velocity-sampler", "gradient-response"});
    require(flowOnly.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::GradientResponse,
            "Gradient-response flow-velocity-sampler should parse before generator validation.");
    requireGeneratorValidationFailure(flowOnly.runOptions.config, "Gradient-response flow without gradient density should be rejected by the generator.");
  }

  void runDensityNormalKappaCompensationValidationRejectsTest() {
    const ParsedRunOptions noneMode = parseArguments(
        {"--flow-velocity-sampler", "density-normal", "--density-evolution", "none", "--density-normal-kappa-compensation"});
    requireGeneratorValidationFailure(
        noneMode.runOptions.config, "Density-normal kappa compensation must be rejected when density-evolution is none.");

    const ParsedRunOptions covarianceMode = parseArguments(
        {"--flow-velocity-sampler", "covariance-ellipse", "--density-evolution", "affine-gaussian", "--density-normal-kappa-compensation"});
    requireGeneratorValidationFailure(
        covarianceMode.runOptions.config, "Density-normal kappa compensation must be rejected outside the density-normal sampler.");

    const ParsedRunOptions gradientMode = parseArguments(
        {"--flow-velocity-sampler", "gradient-response", "--density-evolution", "gradient-response", "--density-normal-kappa-compensation"});
    requireGeneratorValidationFailure(
        gradientMode.runOptions.config, "Density-normal kappa compensation must be rejected for gradient-response mode.");
  }

  void runGradientSigmaOrderingRejectsTest() {
    const ParsedRunOptions parsed = parseArguments({"--density-evolution",
                                                    "gradient-response",
                                                    "--flow-velocity-sampler",
                                                    "gradient-response",
                                                    "--gradient-sigma-em",
                                                    "1.0",
                                                    "--gradient-sigma-dyn",
                                                    "1.0"});
    requireGeneratorValidationFailure(parsed.runOptions.config, "gradient-sigma-dyn <= gradient-sigma-em should be rejected by the generator.");
  }

  void runInvalidGradientVmaxRejectsTest() {
    const ParsedRunOptions parsed = parseArguments({"--density-evolution",
                                                    "gradient-response",
                                                    "--flow-velocity-sampler",
                                                    "gradient-response",
                                                    "--gradient-sigma-em",
                                                    "0.25",
                                                    "--gradient-sigma-dyn",
                                                    "1.25",
                                                    "--gradient-vmax",
                                                    "1.0"});
    requireGeneratorValidationFailure(parsed.runOptions.config, "Invalid gradient-vmax should be rejected by the generator.");
  }

  void runInvalidAffineEvolutionParametersRejectsTest() {
    const ParsedRunOptions invalidLambdaIn = parseArguments({"--affine-lambda-in", "0.0"});
    requireGeneratorValidationFailure(invalidLambdaIn.runOptions.config, "Non-positive affine-lambda-in should be rejected by the generator.");

    const ParsedRunOptions invalidLambdaOut = parseArguments({"--affine-lambda-out", "-0.1"});
    requireGeneratorValidationFailure(invalidLambdaOut.runOptions.config, "Non-positive affine-lambda-out should be rejected by the generator.");

    const ParsedRunOptions invalidSigmaEvo = parseArguments({"--affine-sigma-evo", "-0.01"});
    requireGeneratorValidationFailure(invalidSigmaEvo.runOptions.config, "Negative affine-sigma-evo should be rejected by the generator.");
  }

  void runAffineEffectiveValidationRejectsTest() {
    const ParsedRunOptions invalidDensityMode =
        parseArguments({"--flow-velocity-sampler", "affine-effective", "--density-evolution", "none"});
    requireGeneratorValidationFailure(invalidDensityMode.runOptions.config,
                                      "Affine-effective must be rejected when density-evolution is not affine-gaussian.");

    const ParsedRunOptions invalidDeltaTau =
        parseArguments({"--flow-velocity-sampler", "affine-effective", "--density-evolution", "affine-gaussian", "--affine-delta-tau-ref", "0.0"});
    requireGeneratorValidationFailure(invalidDeltaTau.runOptions.config, "Non-positive affine-delta-tau-ref should be rejected by the generator.");

    const ParsedRunOptions invalidUMax =
        parseArguments({"--flow-velocity-sampler", "affine-effective", "--density-evolution", "affine-gaussian", "--affine-u-max", "1.0"});
    requireGeneratorValidationFailure(invalidUMax.runOptions.config, "Affine-u-max >= 1 should be rejected by the generator.");
  }

  void runDeprecatedRho2RejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--rho2", "0.1"}));
    } catch (const std::invalid_argument &error) {
      threw = std::string(error.what()).find("rho2 -> kappa2") != std::string::npos;
    }
    require(threw, "Deprecated rho2 should be rejected with kappa2 migration guidance.");
  }

  void runDifferentialFlowPtBinParsingAndDefaultModeTest() {
    const ParsedRunOptions parsed = parseArguments({"--v2pt-bins", "0.0, 0.2,0.4, 0.8", "--v3pt-bins", "0.0,0.3,0.9"});
    require(parsed.runOptions.v2PtBinEdges.size() == 4U, "v2pt-bins should parse four edges.");
    requireNear(parsed.runOptions.v2PtBinEdges[0], 0.0, 1.0e-12, "v2pt first edge mismatch.");
    requireNear(parsed.runOptions.v2PtBinEdges[1], 0.2, 1.0e-12, "v2pt second edge mismatch.");
    requireNear(parsed.runOptions.v2PtBinEdges[2], 0.4, 1.0e-12, "v2pt third edge mismatch.");
    requireNear(parsed.runOptions.v2PtBinEdges[3], 0.8, 1.0e-12, "v2pt fourth edge mismatch.");
    require(parsed.runOptions.v3PtBinEdges.size() == 3U, "v3pt-bins should parse three edges.");
    requireNear(parsed.runOptions.v3PtBinEdges[1], 0.3, 1.0e-12, "v3pt second edge mismatch.");
    require(parsed.runOptions.flowPtOutputMode == blastwave::app::FlowPtOutputMode::SameFile,
            "flowpt-output-mode should default to same-file.");
    require(parsed.runOptions.flowPtOutputPath.empty(), "flowpt-output should stay empty by default for same-file mode.");
  }

  void runInvalidDifferentialFlowPtEdgesRejectTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--v2pt-bins", "0.2"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "v2pt-bins must reject lists with fewer than two edges.");

    threw = false;
    try {
      static_cast<void>(parseArguments({"--v2pt-bins", "0.0,0.4,0.4"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "v2pt-bins must reject non-increasing edge lists.");

    threw = false;
    try {
      static_cast<void>(parseArguments({"--v3pt-bins", "0.0,0.3,0.3"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "v3pt-bins must reject non-increasing edge lists.");
  }

  void runFlowPtOutputModeValidationTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--v2pt-bins", "0.0,0.2", "--flowpt-output", "qa/only_allowed_in_separate.root"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "flowpt-output must be rejected in same-file mode.");

    threw = false;
    try {
      static_cast<void>(parseArguments({"--flowpt-output-mode", "separate-file", "--flowpt-output", "qa/no_bins.root"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "flowpt-output-mode separate-file must require v2pt-bins or v3pt-bins.");
  }

  void runFlowPtOutputPathResolutionTest() {
    const TemporaryConfigFile configFile(
        "output = qa/main.root\n"
        "v2pt-bins = 0.0,0.2,0.4\n"
        "v3pt-bins = 0.0,0.3,0.6\n"
        "flowpt-output-mode = separate-file\n"
        "flowpt-output = qa/flowpt_only.root\n");
    const ParsedRunOptions parsed = parseArguments({configFile.path().string()});
    require(parsed.runOptions.flowPtOutputMode == blastwave::app::FlowPtOutputMode::SeparateFile, "flowpt-output-mode parse mismatch.");
    require(parsed.runOptions.flowPtOutputPath == (configFile.path().parent_path() / "qa/flowpt_only.root").lexically_normal().string(),
            "Relative flowpt-output path should resolve against the config directory.");

    const ParsedRunOptions derived = parseArguments({"--output", "qa/main.root", "--v3pt-bins", "0.0,0.2", "--flowpt-output-mode", "separate-file"});
    require(derived.runOptions.flowPtOutputPath == std::filesystem::path("qa/main_flowpt.root").lexically_normal().string(),
            "Separate-file mode should derive default flowpt output path when missing.");
  }

  void runDeprecatedV2PtOutputNamesRejectTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--v2pt-bins", "0.0,0.2", "--v2pt-output-mode", "separate-file"}));
    } catch (const std::invalid_argument &error) {
      threw = std::string(error.what()).find("v2pt-output-mode -> flowpt-output-mode") != std::string::npos;
    }
    require(threw, "Deprecated v2pt-output-mode should be rejected with flowpt migration guidance.");

    threw = false;
    try {
      static_cast<void>(parseArguments({"--v2pt-bins", "0.0,0.2", "--flowpt-output-mode", "separate-file", "--v2pt-output", "qa/old.root"}));
    } catch (const std::invalid_argument &error) {
      threw = std::string(error.what()).find("v2pt-output -> flowpt-output") != std::string::npos;
    }
    require(threw, "Deprecated v2pt-output should be rejected with flowpt migration guidance.");
  }

}  // namespace

int main() {
  try {
    runDefaultSamplerTest();
    runFlowTransCliParseTest();
    runFlowTransConfigAndOverrideParseTest();
    runFlowTransRadiusResolutionParseTest();
    runFlowTransRadiusResolutionCliOverrideParseTest();
    runInvalidFlowTransRadiusResolutionRejectsTest();
    runFlowTransRadiusResolutionValidationRejectsTest();
    runFlowTransMagnitudeModeParseTest();
    runInvalidFlowTransMagnitudeModeRejectsTest();
    runShellGradientCorrectedDensityRadiusValidationTest();
    runShellGradientCorrectedValidationRejectsTest();
    runRadiusProfileGradientKnobValidationRejectsTest();
    runShellGradientBoundValidationRejectsTest();
    runDeprecatedTransverseFlowNamesRejectTest();
    runFlowTransRadiusParseVariantsTest();
    runInvalidFlowTransRadiusRejectsTest();
    runDensityLevelFlowTransValidationRejectsTest();
    runFlowTransValidationRejectsTest();
    runDensityNormalFlowTransValidationSuccessTest();
    runInitialGeometryDefaultParseTest();
    runInitialGeometryFluctuationCliParseTest();
    runInitialGeometryFluctuationConfigParseTest();
    runInitialGeometryFluctuationCliOverrideTest();
    runInitialGeometryFluctuationValidationRejectsTest();
    runCliSamplerAndDensityEvolutionParseTest();
    runInitialGeometryParseAndOverrideTest();
    runInitialGeometryConfigAndOverrideTest();
    runConfigAndOverrideParseTest();
    runInvalidInitialGeometryRejectsTest();
    runGradientConfigParseTest();
    runAffineEffectiveParseTest();
    runAffineEffectiveModeConfigAndOverrideParseTest();
    runGradientCliOverrideParseTest();
    runInvalidSamplerRejectsTest();
    runInvalidDensityEvolutionRejectsTest();
    runInvalidAffineEffectiveModeRejectsTest();
    runInvalidCooperFryeWeightRejectsTest();
    runDensityNormalKappaCompensationConfigParseTest();
    runDensityFlowMismatchRejectsTest();
    runDensityNormalKappaCompensationValidationRejectsTest();
    runGradientSigmaOrderingRejectsTest();
    runInvalidGradientVmaxRejectsTest();
    runInvalidAffineEvolutionParametersRejectsTest();
    runAffineEffectiveValidationRejectsTest();
    runDeprecatedRho2RejectsTest();
    runDifferentialFlowPtBinParsingAndDefaultModeTest();
    runInvalidDifferentialFlowPtEdgesRejectTest();
    runFlowPtOutputModeValidationTest();
    runFlowPtOutputPathResolutionTest();
    runDeprecatedV2PtOutputNamesRejectTest();
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Run options test failed: " << error.what() << '\n';
    return 1;
  }
}
