#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

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

  void runDefaultSamplerTest() {
    const ParsedRunOptions parsed = parseArguments({});
    require(!parsed.showHelp, "Default parse should not request help.");
    require(parsed.runOptions.config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
            "Default flow velocity sampler should stay covariance-ellipse.");
    require(parsed.runOptions.config.densityEvolutionMode == blastwave::DensityEvolutionMode::AffineGaussianResponse,
            "Default density evolution mode should stay affine-gaussian.");
    requireNear(parsed.runOptions.config.kappa2, 1.0986122886681098, 1.0e-12, "Default kappa2 should preserve the previous rho2 numeric default.");
    requireNear(parsed.runOptions.config.flowDensitySigma, 0.5, 1.0e-12, "Default flow density sigma should stay 0.5 fm.");
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
    runInvalidSamplerRejectsTest();
    runInvalidDensityEvolutionRejectsTest();
    runDeprecatedRho2RejectsTest();
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Run options test failed: " << error.what() << '\n';
    return 1;
  }
}
