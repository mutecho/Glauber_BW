#include "generate_blastwave/RunOptions.h"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

  struct DeferredOption {
    std::string name;
    std::string value;
  };

  // Keep migration guidance centralized so deprecated flow keys produce
  // deterministic invalid_argument diagnostics across config and CLI.
  [[noreturn]] void throwDeprecatedFlowOptionError(const std::string &optionName, const std::string &sourceDescription) {
    if (optionName == "vmax") {
      throw std::invalid_argument("Invalid option/key '" + optionName + "' from " + sourceDescription + ". Migration: vmax -> rho0 = atanh(vmax).");
    }
    if (optionName == "rho2") {
      throw std::invalid_argument("Invalid option/key '" + optionName + "' from " + sourceDescription + ". Migration: rho2 -> kappa2.");
    }

    throw std::invalid_argument("Invalid option/key '" + optionName + "' from " + sourceDescription + ". Migration: r-ref -> absorbed by event-ellipse semi-axes.");
  }

  std::string takeValue(int &index, int argc, char **argv, const std::string &optionName) {
    if (index + 1 >= argc) {
      throw std::invalid_argument("Missing value for " + optionName);
    }

    ++index;
    return argv[index];
  }

  std::string trim(const std::string &text) {
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
      return "";
    }

    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
  }

  int parseInt(const std::string &rawValue, const std::string &optionName, const std::string &sourceDescription) {
    std::size_t processedChars = 0;
    try {
      const int parsedValue = std::stoi(rawValue, &processedChars);
      if (processedChars != rawValue.size()) {
        throw std::invalid_argument("trailing characters");
      }
      return parsedValue;
    } catch (const std::exception &) {
      throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription);
    }
  }

  double parseDouble(const std::string &rawValue, const std::string &optionName, const std::string &sourceDescription) {
    std::size_t processedChars = 0;
    try {
      const double parsedValue = std::stod(rawValue, &processedChars);
      if (processedChars != rawValue.size()) {
        throw std::invalid_argument("trailing characters");
      }
      return parsedValue;
    } catch (const std::exception &) {
      throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription);
    }
  }

  std::uint64_t parseUnsignedInteger(const std::string &rawValue, const std::string &optionName, const std::string &sourceDescription) {
    if (!rawValue.empty() && rawValue.front() == '-') {
      throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription);
    }

    std::size_t processedChars = 0;
    try {
      const auto parsedValue = std::stoull(rawValue, &processedChars);
      if (processedChars != rawValue.size()) {
        throw std::invalid_argument("trailing characters");
      }
      return static_cast<std::uint64_t>(parsedValue);
    } catch (const std::exception &) {
      throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription);
    }
  }

  bool parseBool(const std::string &rawValue, const std::string &optionName, const std::string &sourceDescription) {
    if (rawValue == "true") {
      return true;
    }
    if (rawValue == "false") {
      return false;
    }

    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription + ". Expected 'true' or 'false'.");
  }

  blastwave::ThermalSamplerMode parseThermalSamplerMode(const std::string &rawValue, const std::string &optionName, const std::string &sourceDescription) {
    if (rawValue == "maxwell-juttner") {
      return blastwave::ThermalSamplerMode::MaxwellJuttner;
    }
    if (rawValue == "gamma") {
      return blastwave::ThermalSamplerMode::Gamma;
    }

    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription + ". Expected 'maxwell-juttner' or 'gamma'.");
  }

  blastwave::FlowVelocitySamplerMode parseFlowVelocitySamplerMode(const std::string &rawValue, const std::string &optionName, const std::string &sourceDescription) {
    if (rawValue == "covariance-ellipse") {
      return blastwave::FlowVelocitySamplerMode::CovarianceEllipse;
    }
    if (rawValue == "density-normal") {
      return blastwave::FlowVelocitySamplerMode::DensityNormal;
    }
    if (rawValue == "gradient-response") {
      return blastwave::FlowVelocitySamplerMode::GradientResponse;
    }

    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription
                                + ". Expected 'covariance-ellipse', 'density-normal', or 'gradient-response'.");
  }

  blastwave::DensityEvolutionMode parseDensityEvolutionMode(const std::string &rawValue, const std::string &optionName, const std::string &sourceDescription) {
    if (rawValue == "affine-gaussian") {
      return blastwave::DensityEvolutionMode::AffineGaussianResponse;
    }
    if (rawValue == "none") {
      return blastwave::DensityEvolutionMode::None;
    }
    if (rawValue == "gradient-response") {
      return blastwave::DensityEvolutionMode::GradientResponse;
    }

    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription
                                + ". Expected 'affine-gaussian', 'none', or 'gradient-response'.");
  }

  blastwave::CooperFryeWeightMode parseCooperFryeWeightMode(const std::string &rawValue, const std::string &optionName, const std::string &sourceDescription) {
    if (rawValue == "none") {
      return blastwave::CooperFryeWeightMode::None;
    }
    if (rawValue == "mt-cosh") {
      return blastwave::CooperFryeWeightMode::MtCosh;
    }

    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" + optionName + "' from " + sourceDescription + ". Expected 'none' or 'mt-cosh'.");
  }

  std::string resolveOutputPath(const std::string &rawValue, const std::filesystem::path &baseDirectory) {
    if (rawValue.empty()) {
      throw std::invalid_argument("Output path must not be empty.");
    }

    const std::filesystem::path outputPath(rawValue);
    if (outputPath.is_absolute()) {
      return outputPath.lexically_normal().string();
    }
    return (baseDirectory / outputPath).lexically_normal().string();
  }

  // Apply one config or CLI key onto the runtime surface so precedence stays
  // explicit and consistent between file-driven and flag-driven execution.
  void applyOption(blastwave::app::RunOptions &runOptions,
                   const std::string &optionName,
                   const std::string &rawValue,
                   const std::string &sourceDescription,
                   const std::filesystem::path &baseDirectory) {
    if (optionName == "nevents") {
      runOptions.config.nEvents = parseInt(rawValue, optionName, sourceDescription);
    } else if (optionName == "b") {
      runOptions.config.impactParameter = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "temperature") {
      runOptions.config.temperature = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "thermal-sampler") {
      runOptions.config.thermalSamplerMode = parseThermalSamplerMode(rawValue, optionName, sourceDescription);
    } else if (optionName == "mj-pmax") {
      runOptions.config.mjPMax = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "mj-grid-points") {
      runOptions.config.mjGridPoints = parseInt(rawValue, optionName, sourceDescription);
    } else if (optionName == "tau0") {
      runOptions.config.tau0 = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "smear") {
      runOptions.config.smearSigma = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "sigma-nn") {
      runOptions.config.sigmaNN = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "seed") {
      runOptions.config.seed = parseUnsignedInteger(rawValue, optionName, sourceDescription);
    } else if (optionName == "output") {
      runOptions.outputPath = resolveOutputPath(rawValue, baseDirectory);
    } else if (optionName == "progress") {
      runOptions.progressMode = parseBool(rawValue, optionName, sourceDescription) ? blastwave::app::ProgressMode::Enabled : blastwave::app::ProgressMode::Disabled;
    } else if (optionName == "rho0") {
      runOptions.config.rho0 = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "kappa2") {
      runOptions.config.kappa2 = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "flow-power") {
      runOptions.config.flowPower = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "flow-velocity-sampler") {
      runOptions.config.flowVelocitySamplerMode = parseFlowVelocitySamplerMode(rawValue, optionName, sourceDescription);
    } else if (optionName == "density-evolution") {
      runOptions.config.densityEvolutionMode = parseDensityEvolutionMode(rawValue, optionName, sourceDescription);
    } else if (optionName == "flow-density-sigma") {
      runOptions.config.flowDensitySigma = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "debug-flow-ellipse") {
      runOptions.config.debugFlowEllipse = parseBool(rawValue, optionName, sourceDescription);
    } else if (optionName == "debug-gradient-response") {
      runOptions.config.debugGradientResponse = parseBool(rawValue, optionName, sourceDescription);
    } else if (optionName == "sigma-eta") {
      runOptions.config.sigmaEta = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "eta-plateau") {
      runOptions.config.etaPlateauHalfWidth = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "nbd-mu") {
      runOptions.config.nbdMu = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "nbd-k") {
      runOptions.config.nbdK = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-sigma-em") {
      runOptions.config.gradientSigmaEm = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-sigma-dyn") {
      runOptions.config.gradientSigmaDyn = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-density-floor-fraction") {
      runOptions.config.gradientDensityFloorFraction = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-density-cutoff-fraction") {
      runOptions.config.gradientDensityCutoffFraction = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-displacement-max") {
      runOptions.config.gradientDisplacementMax = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-displacement-kappa") {
      runOptions.config.gradientDisplacementKappa = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-diffusion-sigma") {
      runOptions.config.gradientDiffusionSigma = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-vmax") {
      runOptions.config.gradientVMax = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "gradient-velocity-kappa") {
      runOptions.config.gradientVelocityKappa = parseDouble(rawValue, optionName, sourceDescription);
    } else if (optionName == "cooper-frye-weight") {
      runOptions.config.cooperFryeWeightMode = parseCooperFryeWeightMode(rawValue, optionName, sourceDescription);
    } else if (optionName == "vmax" || optionName == "rho2" || optionName == "r-ref") {
      throwDeprecatedFlowOptionError(optionName, sourceDescription);
    } else {
      throw std::invalid_argument("Unknown option/key '" + optionName + "' from " + sourceDescription);
    }
  }

  // Parse the lightweight key=value configuration format and apply it before
  // CLI overrides so file defaults and explicit flags keep a stable precedence.
  void loadConfigFile(const std::string &configPathString, blastwave::app::RunOptions &runOptions) {
    const std::filesystem::path configPath(configPathString);
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
      throw std::runtime_error("Failed to open configuration file: " + configPathString);
    }

    std::unordered_set<std::string> seenKeys;
    std::string line;
    int lineNumber = 0;
    while (std::getline(configFile, line)) {
      ++lineNumber;

      const std::string trimmedLine = trim(line);
      if (trimmedLine.empty() || trimmedLine.front() == '#') {
        continue;
      }

      const std::size_t separator = trimmedLine.find('=');
      if (separator == std::string::npos) {
        throw std::invalid_argument("Invalid configuration line " + std::to_string(lineNumber) + " in " + configPathString + ": expected key=value");
      }

      const std::string key = trim(trimmedLine.substr(0, separator));
      const std::string value = trim(trimmedLine.substr(separator + 1));
      if (key.empty()) {
        throw std::invalid_argument("Empty configuration key on line " + std::to_string(lineNumber) + " in " + configPathString);
      }
      if (!seenKeys.insert(key).second) {
        throw std::invalid_argument("Duplicate configuration key '" + key + "' on line " + std::to_string(lineNumber) + " in " + configPathString);
      }

      applyOption(runOptions, key, value, "configuration file '" + configPathString + "' line " + std::to_string(lineNumber), configPath.parent_path());
    }
  }

}  // namespace

namespace blastwave::app {

  void printGenerateUsage(const char *programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "       " << programName << " --config <path> [options]\n"
              << "       " << programName << " <config-path> [options]\n"
              << "Option precedence:\n"
              << "  explicit CLI options > configuration file values > built-in defaults\n"
              << "Configuration file format:\n"
              << "  - plain text with one 'key = value' entry per line\n"
              << "  - blank lines and full-line '#' comments are ignored\n"
              << "  - relative output paths in config files are resolved against the\n"
              << "    config file directory\n"
              << "Minimal config example:\n"
              << "  nevents = 5000\n"
              << "  b = 8\n"
              << "  progress = true\n"
              << "  output = qa/test_b8_5000.root\n"
              << "Configuration keys:\n"
              << "  nevents, b, temperature, thermal-sampler, mj-pmax, mj-grid-points,\n"
              << "  tau0, smear, sigma-nn, seed, output, progress,\n"
              << "  rho0, kappa2, flow-power, flow-velocity-sampler, density-evolution,\n"
              << "  flow-density-sigma,\n"
              << "  gradient-sigma-em, gradient-sigma-dyn,\n"
              << "  gradient-density-floor-fraction, gradient-density-cutoff-fraction,\n"
              << "  gradient-displacement-max, gradient-displacement-kappa,\n"
              << "  gradient-diffusion-sigma, gradient-vmax, gradient-velocity-kappa,\n"
              << "  cooper-frye-weight,\n"
              << "  debug-flow-ellipse, debug-gradient-response,\n"
              << "  sigma-eta, eta-plateau, nbd-mu, nbd-k\n"
              << "Primary options:\n"
              << "  --nevents <int>\n"
              << "  --b <fm>\n"
              << "  --temperature <GeV>\n"
              << "  --thermal-sampler <maxwell-juttner|gamma>\n"
              << "  --mj-pmax <GeV>\n"
              << "  --mj-grid-points <int>\n"
              << "  --tau0 <fm/c>\n"
              << "  --smear <fm>\n"
              << "  --sigma-nn <fm^2>   default 7.0 fm^2 (70 mb)\n"
              << "  --seed <uint64>\n"
              << "  --output <path>\n"
              << "  --progress\n"
              << "  --no-progress\n"
              << "  --debug-flow-ellipse\n"
              << "  --no-debug-flow-ellipse\n"
              << "  --debug-gradient-response\n"
              << "  --no-debug-gradient-response\n"
              << "QA-facing tuning knobs:\n"
              << "  (gradient-response requires matching density/flow modes)\n"
              << "  --rho0 <value>\n"
              << "  --kappa2 <value>\n"
              << "  --flow-power <value>\n"
              << "  --flow-velocity-sampler <covariance-ellipse|density-normal|gradient-response>\n"
              << "  --density-evolution <affine-gaussian|none|gradient-response>\n"
              << "  --flow-density-sigma <fm>\n"
              << "  --gradient-sigma-em <fm>\n"
              << "  --gradient-sigma-dyn <fm>\n"
              << "  --gradient-density-floor-fraction <value>\n"
              << "  --gradient-density-cutoff-fraction <value>\n"
              << "  --gradient-displacement-max <fm>\n"
              << "  --gradient-displacement-kappa <fm>\n"
              << "  --gradient-diffusion-sigma <fm>\n"
              << "  --gradient-vmax <value>\n"
              << "  --gradient-velocity-kappa <fm>\n"
              << "  --cooper-frye-weight <none|mt-cosh>\n"
              << "  --sigma-eta <value>\n"
              << "  --eta-plateau <value>\n"
              << "  --nbd-mu <value>\n"
              << "  --nbd-k <value>\n"
              << "  --help\n";
  }

  // Parse both config-file and pure-CLI entry styles while preserving the
  // explicit precedence contract documented for the generator interface.
  RunOptions parseRunOptions(int argc, char **argv, bool &showHelp) {
    RunOptions runOptions;
    std::vector<DeferredOption> cliOverrides;
    std::string flagConfigPath;
    std::string positionalConfigPath;
    ProgressMode cliProgressMode = ProgressMode::Auto;
    bool hasCliProgressOverride = false;
    bool cliDebugFlowEllipse = false;
    bool hasCliDebugFlowEllipseOverride = false;
    bool cliDebugGradientResponse = false;
    bool hasCliDebugGradientResponseOverride = false;

    showHelp = false;

    for (int iArg = 1; iArg < argc; ++iArg) {
      const std::string argument = argv[iArg];
      if (argument == "--help") {
        showHelp = true;
        return runOptions;
      }

      if (argument == "--config") {
        if (!flagConfigPath.empty()) {
          throw std::invalid_argument("Only one --config path may be provided.");
        }
        flagConfigPath = takeValue(iArg, argc, argv, argument);
        continue;
      }

      if (argument == "--progress") {
        cliProgressMode = ProgressMode::Enabled;
        hasCliProgressOverride = true;
        continue;
      }

      if (argument == "--no-progress") {
        cliProgressMode = ProgressMode::Disabled;
        hasCliProgressOverride = true;
        continue;
      }

      if (argument == "--debug-flow-ellipse") {
        cliDebugFlowEllipse = true;
        hasCliDebugFlowEllipseOverride = true;
        continue;
      }

      if (argument == "--no-debug-flow-ellipse") {
        cliDebugFlowEllipse = false;
        hasCliDebugFlowEllipseOverride = true;
        continue;
      }

      if (argument == "--debug-gradient-response") {
        cliDebugGradientResponse = true;
        hasCliDebugGradientResponseOverride = true;
        continue;
      }

      if (argument == "--no-debug-gradient-response") {
        cliDebugGradientResponse = false;
        hasCliDebugGradientResponseOverride = true;
        continue;
      }

      if (argument.rfind("--", 0) == 0) {
        cliOverrides.push_back({argument.substr(2), takeValue(iArg, argc, argv, argument)});
        continue;
      }

      if (!positionalConfigPath.empty()) {
        throw std::invalid_argument("Only one positional configuration file path is allowed.");
      }
      positionalConfigPath = argument;
    }

    if (!flagConfigPath.empty() && !positionalConfigPath.empty()) {
      throw std::invalid_argument("Cannot use both a positional configuration file path and --config.");
    }

    const std::string configPath = !flagConfigPath.empty() ? flagConfigPath : positionalConfigPath;
    if (!configPath.empty()) {
      loadConfigFile(configPath, runOptions);
    }

    for (const DeferredOption &overrideOption : cliOverrides) {
      applyOption(runOptions, overrideOption.name, overrideOption.value, "command line option '--" + overrideOption.name + "'", std::filesystem::path());
    }

    if (hasCliProgressOverride) {
      runOptions.progressMode = cliProgressMode;
    }
    if (hasCliDebugFlowEllipseOverride) {
      runOptions.config.debugFlowEllipse = cliDebugFlowEllipse;
    }
    if (hasCliDebugGradientResponseOverride) {
      runOptions.config.debugGradientResponse = cliDebugGradientResponse;
    }

    return runOptions;
  }

}  // namespace blastwave::app
