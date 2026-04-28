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

  void runDefaultSamplerTest() {
    const ParsedRunOptions parsed = parseArguments({});
    require(!parsed.showHelp, "Default parse should not request help.");
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
            "Default flow velocity sampler should stay covariance-ellipse.");
    require(parsed.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse,
            "Default density evolution mode should stay affine-gaussian.");
    requireNear(parsed.runOptions.config.kappa2, 1.0986122886681098, 1.0e-12, "Default kappa2 should preserve the previous rho2 numeric default.");
    requireNear(parsed.runOptions.config.flowDensitySigma, 0.5, 1.0e-12, "Default flow density sigma should stay 0.5 fm.");
    requireNear(parsed.runOptions.config.gradientSigmaEm, 0.0, 1.0e-12, "Default gradient sigma em should stay disabled.");
    requireNear(parsed.runOptions.config.gradientSigmaDyn, 1.0, 1.0e-12, "Default gradient sigma dyn should stay 1.0 fm.");
    requireNear(parsed.runOptions.config.gradientDensityFloorFraction, 1.0e-4, 1.0e-16, "Default gradient density floor fraction mismatch.");
    requireNear(parsed.runOptions.config.gradientDensityCutoffFraction, 1.0e-6, 1.0e-18, "Default gradient density cutoff fraction mismatch.");
    requireNear(parsed.runOptions.config.gradientDisplacementMax, 1.5, 1.0e-12, "Default gradient displacement max mismatch.");
    requireNear(parsed.runOptions.config.gradientDisplacementKappa, 1.0, 1.0e-12, "Default gradient displacement kappa mismatch.");
    requireNear(parsed.runOptions.config.gradientDiffusionSigma, 0.0, 1.0e-12, "Default gradient diffusion sigma should stay disabled.");
    requireNear(parsed.runOptions.config.gradientVMax, 0.75, 1.0e-12, "Default gradient vmax mismatch.");
    requireNear(parsed.runOptions.config.gradientVelocityKappa, 1.0, 1.0e-12, "Default gradient velocity kappa mismatch.");
    require(!parsed.runOptions.config.debugGradientResponse, "Debug gradient response should stay disabled by default.");
    require(parsed.runOptions.config.cooperFryeWeightMode == blastwave::CooperFryeWeightMode::None,
            "Cooper-Frye weight should default to none.");
  }

  void runCliSamplerAndDensityEvolutionParseTest() {
    const ParsedRunOptions parsed = parseArguments(
        {"--flow-velocity-sampler", "density-normal", "--density-evolution", "none", "--kappa2", "0.25", "--flow-density-sigma", "0.8"});
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::DensityNormal,
            "CLI sampler parse should accept density-normal.");
    require(parsed.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::None,
            "CLI density-evolution parse should accept none.");
    requireNear(parsed.runOptions.config.kappa2, 0.25, 1.0e-12, "CLI kappa2 parse mismatch.");
    requireNear(parsed.runOptions.config.flowDensitySigma, 0.8, 1.0e-12, "CLI flow density sigma parse mismatch.");
  }

  void runConfigAndOverrideParseTest() {
    const TemporaryConfigFile configFile(
        "flow-velocity-sampler = density-normal\n"
        "density-evolution = none\n"
        "kappa2 = 0.3\n"
        "flow-density-sigma = 0.7\n"
        "output = qa/test.root\n");
    const ParsedRunOptions parsed = parseArguments(
        {configFile.path().string(), "--flow-velocity-sampler", "covariance-ellipse", "--density-evolution", "affine-gaussian", "--kappa2", "0.45"});
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
            "CLI override should win over config-file sampler value.");
    require(parsed.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse,
            "CLI override should win over config-file density-evolution value.");
    requireNear(parsed.runOptions.config.kappa2, 0.45, 1.0e-12, "CLI override should win over config-file kappa2 value.");
    requireNear(parsed.runOptions.config.flowDensitySigma, 0.7, 1.0e-12, "Config-file flow density sigma parse mismatch.");
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

  void runInvalidCooperFryeWeightRejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--cooper-frye-weight", "not-a-weight"}));
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "Invalid cooper-frye-weight should be rejected.");
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

  void runDeprecatedRho2RejectsTest() {
    bool threw = false;
    try {
      static_cast<void>(parseArguments({"--rho2", "0.1"}));
    } catch (const std::invalid_argument &error) {
      threw = std::string(error.what()).find("rho2 -> kappa2") != std::string::npos;
    }
    require(threw, "Deprecated rho2 should be rejected with kappa2 migration guidance.");
  }

}  // namespace

int main() {
  try {
    runDefaultSamplerTest();
    runCliSamplerAndDensityEvolutionParseTest();
    runConfigAndOverrideParseTest();
    runGradientConfigParseTest();
    runGradientCliOverrideParseTest();
    runInvalidSamplerRejectsTest();
    runInvalidDensityEvolutionRejectsTest();
    runInvalidCooperFryeWeightRejectsTest();
    runDensityFlowMismatchRejectsTest();
    runGradientSigmaOrderingRejectsTest();
    runInvalidGradientVmaxRejectsTest();
    runDeprecatedRho2RejectsTest();
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Run options test failed: " << error.what() << '\n';
    return 1;
  }
}
