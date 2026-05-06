#include <TFile.h>
#include <TTree.h>

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "blastwave/DifferentialFlowCumulant.h"
#include "blastwave/io/DifferentialFlowRootPayload.h"
#include "blastwave/io/OutputPathUtils.h"
#include "blastwave/io/RootOutputSchema.h"

namespace {

  struct Options {
    std::string inputPath;
    std::string outputPath;
    bool inplace = false;
  };

  void printUsage(const char *programName) {
    std::cout << "Usage: " << programName << " --input <result.root> [--output <analysis.root> | --inplace]\n";
  }

  std::string takeValue(int &index, int argc, char **argv, const std::string &optionName) {
    if (index + 1 >= argc) {
      throw std::invalid_argument("Missing value for " + optionName);
    }
    ++index;
    return argv[index];
  }

  std::string deriveDefaultOutputPath(const std::string &inputPath) {
    const std::filesystem::path input(inputPath);
    return (input.parent_path() / (input.stem().string() + "_flowpt.root")).lexically_normal().string();
  }

  Options parseOptions(int argc, char **argv) {
    Options options;
    bool sawOutput = false;
    for (int iArg = 1; iArg < argc; ++iArg) {
      const std::string option = argv[iArg];
      if (option == "--help") {
        printUsage(argv[0]);
        std::exit(0);
      } else if (option == "--input") {
        options.inputPath = takeValue(iArg, argc, argv, option);
      } else if (option == "--output") {
        options.outputPath = takeValue(iArg, argc, argv, option);
        sawOutput = true;
      } else if (option == "--inplace") {
        options.inplace = true;
      } else {
        throw std::invalid_argument("Unknown option: " + option);
      }
    }

    if (options.inputPath.empty()) {
      throw std::invalid_argument("--input is required.");
    }
    if (options.inplace && sawOutput) {
      throw std::invalid_argument("--output and --inplace are mutually exclusive.");
    }
    if (!options.inplace) {
      options.outputPath = sawOutput ? options.outputPath : deriveDefaultOutputPath(options.inputPath);
      if (options.outputPath.empty()) {
        throw std::invalid_argument("Output path must not be empty.");
      }
    }
    return options;
  }

  std::string summarizeHarmonics(const std::vector<blastwave::DifferentialFlowCumulantResult> &results) {
    std::ostringstream stream;
    for (std::size_t iResult = 0; iResult < results.size(); ++iResult) {
      if (iResult > 0U) {
        stream << ",";
      }
      stream << results[iResult].harmonic << ":" << results[iResult].values.size();
    }
    return stream.str();
  }

}  // namespace

int main(int argc, char **argv) {
  try {
    const Options options = parseOptions(argc, argv);

    TFile inputFile(options.inputPath.c_str(), "READ");
    if (inputFile.IsZombie()) {
      throw std::runtime_error("Failed to open input ROOT file: " + options.inputPath);
    }
    auto *particlesTree = dynamic_cast<TTree *>(inputFile.Get(blastwave::io::kParticlesTreeName));
    if (particlesTree == nullptr) {
      throw std::runtime_error("Missing particles tree 'particles' in input ROOT file.");
    }

    std::vector<std::pair<int, std::vector<double>>> configuredHarmonics;
    for (int harmonic : {2, 3}) {
      const std::optional<std::vector<double>> ptBinEdges = blastwave::io::readDifferentialFlowEdges(inputFile, harmonic);
      if (ptBinEdges.has_value()) {
        configuredHarmonics.push_back({harmonic, *ptBinEdges});
      }
    }
    if (configuredHarmonics.empty()) {
      throw std::runtime_error("No differential-flow pT edge metadata found. Expected 'v2_2_pt_edges' or 'v3_2_pt_edges'.");
    }

    blastwave::io::ParticleBranches particleBranches;
    blastwave::io::bindParticleBranches(*particlesTree, particleBranches);

    std::unordered_map<int, std::vector<blastwave::DifferentialFlowTrack>> tracksByEvent;
    for (Long64_t iEntry = 0; iEntry < particlesTree->GetEntries(); ++iEntry) {
      particlesTree->GetEntry(iEntry);
      tracksByEvent[particleBranches.eventId].push_back({particleBranches.px, particleBranches.py});
    }

    std::vector<int> eventIds;
    eventIds.reserve(tracksByEvent.size());
    for (const auto &entry : tracksByEvent) {
      eventIds.push_back(entry.first);
    }
    std::sort(eventIds.begin(), eventIds.end());

    std::vector<blastwave::DifferentialFlowCumulantResult> results;
    results.reserve(configuredHarmonics.size());
    for (const auto &configuredHarmonic : configuredHarmonics) {
      blastwave::DifferentialFlowCumulant cumulant(configuredHarmonic.first, configuredHarmonic.second);
      for (int eventId : eventIds) {
        cumulant.addEvent(tracksByEvent[eventId]);
      }
      results.push_back(cumulant.finalize());
    }
    inputFile.Close();

    if (options.inplace) {
      TFile outputFile(options.inputPath.c_str(), "UPDATE");
      if (outputFile.IsZombie()) {
        throw std::runtime_error("Failed to open input ROOT file for in-place update: " + options.inputPath);
      }
      for (const blastwave::DifferentialFlowCumulantResult &result : results) {
        blastwave::io::writeDifferentialFlowPayload(outputFile, result);
      }
      outputFile.Close();
      std::cout << "wrote_flowpt_payload input=" << options.inputPath << " mode=inplace harmonics=" << summarizeHarmonics(results) << '\n';
      return 0;
    }

    blastwave::io::ensureOutputDirectoryExists(options.outputPath, std::cout);
    TFile outputFile(options.outputPath.c_str(), "RECREATE");
    if (outputFile.IsZombie()) {
      throw std::runtime_error("Failed to create output ROOT file: " + options.outputPath);
    }
    for (const blastwave::DifferentialFlowCumulantResult &result : results) {
      blastwave::io::writeDifferentialFlowPayload(outputFile, result);
    }
    outputFile.Close();
    std::cout << "wrote_flowpt_payload input=" << options.inputPath << " output=" << options.outputPath << " mode=separate-file harmonics=" << summarizeHarmonics(results)
              << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "analyze_blastwave_vnpt failed: " << error.what() << '\n';
    return 1;
  }
}
