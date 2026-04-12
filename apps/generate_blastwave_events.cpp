#include "blastwave/BlastWaveGenerator.h"
#include "blastwave/io/RootOutputSchema.h"

#include <TCanvas.h>
#include <TEllipse.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLegend.h>
#include <TPaveStats.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
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

struct RunOptions {
  blastwave::BlastWaveConfig config;
  std::string outputPath = "blastwave.root";
};

struct DeferredOption {
  std::string name;
  std::string value;
};

void printUsage(const char *programName) {
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
            << "  output = qa/test_b8_5000.root\n"
            << "Configuration keys:\n"
            << "  nevents, b, temperature, tau0, smear, sigma-nn, seed, output,\n"
            << "  vmax, kappa2, sigma-eta, eta-plateau, nbd-mu, nbd-k, r-ref\n"
            << "Primary options:\n"
            << "  --nevents <int>\n"
            << "  --b <fm>\n"
            << "  --temperature <GeV>\n"
            << "  --tau0 <fm/c>\n"
            << "  --smear <fm>\n"
            << "  --sigma-nn <fm^2>   default 7.0 fm^2 (70 mb)\n"
            << "  --seed <uint64>\n"
            << "  --output <path>\n"
            << "QA-facing tuning knobs:\n"
            << "  --vmax <value>\n"
            << "  --kappa2 <value>\n"
            << "  --sigma-eta <value>\n"
            << "  --eta-plateau <value>\n"
            << "  --nbd-mu <value>\n"
            << "  --nbd-k <value>\n"
            << "  --r-ref <fm>\n"
            << "  --help\n";
}

// eta = ln (|p|+pz)/(|p|-pz)
// 雁快度提取
double computePseudorapidity(double px, double py, double pz) {
  const double momentumMagnitude = std::sqrt(px * px + py * py + pz * pz);
  const double denominator = momentumMagnitude - pz;
  if (denominator <= 1.0e-9) {
    return (pz >= 0.0) ? 10.0 : -10.0;
  }
  return 0.5 * std::log((momentumMagnitude + pz) / denominator);
}

std::string takeValue(int &index, int argc, char **argv,
                      const std::string &optionName) {
  if (index + 1 >= argc) {
    throw std::invalid_argument("Missing value for " + optionName);
  }
  ++index;
  return argv[index];
}

std::string trim(const std::string& text) {
  const std::size_t first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return "";
  }
  const std::size_t last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1);
}

int parseInt(const std::string& rawValue, const std::string& optionName,
             const std::string& sourceDescription) {
  std::size_t processedChars = 0;
  try {
    const int parsedValue = std::stoi(rawValue, &processedChars);
    if (processedChars != rawValue.size()) {
      throw std::invalid_argument("trailing characters");
    }
    return parsedValue;
  } catch (const std::exception&) {
    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" +
                                optionName + "' from " + sourceDescription);
  }
}

double parseDouble(const std::string& rawValue, const std::string& optionName,
                   const std::string& sourceDescription) {
  std::size_t processedChars = 0;
  try {
    const double parsedValue = std::stod(rawValue, &processedChars);
    if (processedChars != rawValue.size()) {
      throw std::invalid_argument("trailing characters");
    }
    return parsedValue;
  } catch (const std::exception&) {
    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" +
                                optionName + "' from " + sourceDescription);
  }
}

std::uint64_t parseUnsignedInteger(const std::string& rawValue,
                                   const std::string& optionName,
                                   const std::string& sourceDescription) {
  if (!rawValue.empty() && rawValue.front() == '-') {
    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" +
                                optionName + "' from " + sourceDescription);
  }

  std::size_t processedChars = 0;
  try {
    const auto parsedValue = std::stoull(rawValue, &processedChars);
    if (processedChars != rawValue.size()) {
      throw std::invalid_argument("trailing characters");
    }
    return static_cast<std::uint64_t>(parsedValue);
  } catch (const std::exception&) {
    throw std::invalid_argument("Invalid value '" + rawValue + "' for '" +
                                optionName + "' from " + sourceDescription);
  }
}

std::string resolveOutputPath(const std::string& rawValue,
                              const std::filesystem::path& baseDirectory) {
  if (rawValue.empty()) {
    throw std::invalid_argument("Output path must not be empty.");
  }

  const std::filesystem::path outputPath(rawValue);
  if (outputPath.is_absolute()) {
    return outputPath.lexically_normal().string();
  }
  return (baseDirectory / outputPath).lexically_normal().string();
}

void applyOption(RunOptions& runOptions, const std::string& optionName,
                 const std::string& rawValue,
                 const std::string& sourceDescription,
                 const std::filesystem::path& baseDirectory) {
  if (optionName == "nevents") {
    runOptions.config.nEvents =
        parseInt(rawValue, optionName, sourceDescription);
  } else if (optionName == "b") {
    runOptions.config.impactParameter =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "temperature") {
    runOptions.config.temperature =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "tau0") {
    runOptions.config.tau0 =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "smear") {
    runOptions.config.smearSigma =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "sigma-nn") {
    runOptions.config.sigmaNN =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "seed") {
    runOptions.config.seed =
        parseUnsignedInteger(rawValue, optionName, sourceDescription);
  } else if (optionName == "output") {
    runOptions.outputPath = resolveOutputPath(rawValue, baseDirectory);
  } else if (optionName == "vmax") {
    runOptions.config.vMax =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "kappa2") {
    runOptions.config.kappa2 =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "sigma-eta") {
    runOptions.config.sigmaEta =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "eta-plateau") {
    runOptions.config.etaPlateauHalfWidth =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "nbd-mu") {
    runOptions.config.nbdMu =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "nbd-k") {
    runOptions.config.nbdK =
        parseDouble(rawValue, optionName, sourceDescription);
  } else if (optionName == "r-ref") {
    runOptions.config.referenceRadius =
        parseDouble(rawValue, optionName, sourceDescription);
  } else {
    throw std::invalid_argument("Unknown option/key '" + optionName +
                                "' from " + sourceDescription);
  }
}

void loadConfigFile(const std::string& configPathString, RunOptions& runOptions) {
  const std::filesystem::path configPath(configPathString);
  std::ifstream configFile(configPath);
  if (!configFile.is_open()) {
    throw std::runtime_error("Failed to open configuration file: " +
                             configPathString);
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
      throw std::invalid_argument("Invalid configuration line " +
                                  std::to_string(lineNumber) + " in " +
                                  configPathString + ": expected key=value");
    }

    const std::string key = trim(trimmedLine.substr(0, separator));
    const std::string value = trim(trimmedLine.substr(separator + 1));
    if (key.empty()) {
      throw std::invalid_argument("Empty configuration key on line " +
                                  std::to_string(lineNumber) + " in " +
                                  configPathString);
    }
    if (!seenKeys.insert(key).second) {
      throw std::invalid_argument("Duplicate configuration key '" + key +
                                  "' on line " + std::to_string(lineNumber) +
                                  " in " + configPathString);
    }

    applyOption(runOptions, key, value,
                "configuration file '" + configPathString + "' line " +
                    std::to_string(lineNumber),
                configPath.parent_path());
  }
}

RunOptions parseRunOptions(int argc, char** argv, bool& showHelp) {
  RunOptions runOptions;
  std::vector<DeferredOption> cliOverrides;
  std::string flagConfigPath;
  std::string positionalConfigPath;

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

    if (argument.rfind("--", 0) == 0) {
      cliOverrides.push_back(
          {argument.substr(2), takeValue(iArg, argc, argv, argument)});
      continue;
    }

    if (!positionalConfigPath.empty()) {
      throw std::invalid_argument(
          "Only one positional configuration file path is allowed.");
    }
    positionalConfigPath = argument;
  }

  if (!flagConfigPath.empty() && !positionalConfigPath.empty()) {
    throw std::invalid_argument(
        "Cannot use both a positional configuration file path and --config.");
  }

  const std::string configPath =
      !flagConfigPath.empty() ? flagConfigPath : positionalConfigPath;
  if (!configPath.empty()) {
    loadConfigFile(configPath, runOptions);
  }

  for (const DeferredOption& overrideOption : cliOverrides) {
    applyOption(runOptions, overrideOption.name, overrideOption.value,
                "command line option '--" + overrideOption.name + "'",
                std::filesystem::path());
  }

  return runOptions;
}

double
computeParticipantDisplayExtent(const blastwave::BlastWaveConfig &config) {
  const double diffusePadding = 2.0 * config.woodsSaxonDiffuseness + 1.0;
  const double halfSpan = 0.5 * std::abs(config.impactParameter) +
                          config.woodsSaxonRadius + diffusePadding;
  return std::max(15.0, std::ceil(halfSpan));
}

blastwave::io::EventBranches toEventBranches(const blastwave::EventInfo& info) {
  blastwave::io::EventBranches branches;
  branches.eventId = static_cast<Int_t>(info.eventId);
  branches.impactParameter = info.impactParameter;
  branches.nParticipants = static_cast<Int_t>(info.nParticipants);
  branches.eps2 = info.eps2;
  branches.psi2 = info.psi2;
  branches.centrality = info.centrality;
  branches.nCharged = static_cast<Int_t>(info.nCharged);
  return branches;
}

blastwave::io::ParticipantBranches toParticipantBranches(
    const blastwave::ParticipantRecord& participant) {
  blastwave::io::ParticipantBranches branches;
  branches.eventId = static_cast<Int_t>(participant.eventId);
  branches.nucleusId = static_cast<Int_t>(participant.nucleusId);
  branches.x = participant.x;
  branches.y = participant.y;
  branches.z = participant.z;
  return branches;
}

blastwave::io::ParticleBranches toParticleBranches(
    const blastwave::ParticleRecord& particle) {
  blastwave::io::ParticleBranches branches;
  branches.eventId = static_cast<Int_t>(particle.eventId);
  branches.pid = static_cast<Int_t>(particle.pid);
  branches.charge = static_cast<Int_t>(particle.charge);
  branches.mass = particle.mass;
  branches.x = particle.x;
  branches.y = particle.y;
  branches.z = particle.z;
  branches.t = particle.t;
  branches.px = particle.px;
  branches.py = particle.py;
  branches.pz = particle.pz;
  branches.etaS = particle.etaS;
  branches.sourceX = particle.sourceX;
  branches.sourceY = particle.sourceY;
  // Preserve the current on-disk contract: the serialized energy is derived
  // from the branch mass and momentum components at write time.
  branches.energy =
      std::sqrt(branches.mass * branches.mass + branches.px * branches.px +
                branches.py * branches.py + branches.pz * branches.pz);
  return branches;
}

} // namespace

int main(int argc, char **argv) {
  try {
    bool showHelp = false;
    const RunOptions runOptions = parseRunOptions(argc, argv, showHelp);
    if (showHelp) {
      printUsage(argv[0]);
      return 0;
    }

    const blastwave::BlastWaveConfig& config = runOptions.config;
    const std::string& outputPath = runOptions.outputPath;
    blastwave::BlastWaveGenerator generator(config);

    gROOT->SetBatch(true);
    gStyle->SetOptStat(1110);

    TFile outputFile(outputPath.c_str(), "RECREATE");
    if (outputFile.IsZombie()) {
      throw std::runtime_error("Failed to create ROOT output file: " +
                               outputPath);
    }

    TTree eventsTree(blastwave::io::kEventsTreeName, "Blast-wave event summary");
    TTree participantsTree(blastwave::io::kParticipantsTreeName,
                           "Participant nucleon records");
    TTree particlesTree(blastwave::io::kParticlesTreeName,
                        "Blast-wave particle records");

    blastwave::io::EventBranches eventBranches;
    blastwave::io::ParticipantBranches participantBranches;
    blastwave::io::ParticleBranches particleBranches;

    blastwave::io::declareEventBranches(eventsTree, eventBranches);
    blastwave::io::declareParticipantBranches(participantsTree,
                                              participantBranches);
    blastwave::io::declareParticleBranches(particlesTree, particleBranches);

    const double participantDisplayExtent =
        computeParticipantDisplayExtent(config);
    TH1F hNpart(blastwave::io::kNpartHistogramName,
                "Participant multiplicity;Npart;Events", 400, -0.5, 399.5);
    TH1F hEps2(blastwave::io::kEps2HistogramName,
               "Participant eccentricity;#epsilon_{2};Events", 100, 0.0, 1.0);
    TH1F hPsi2(blastwave::io::kPsi2HistogramName,
               "Participant-plane angle;#Psi_{2} [rad];Events", 128, -1.7,
               1.7);
    TH1F hCentrality(blastwave::io::kCentralityHistogramName,
                     "Centrality;centrality [%];Events", 11, 0.0, 110.0);
    TH2F hParticipantXY(blastwave::io::kParticipantXYHistogramName,
                        "Participant nucleons;x [fm];y [fm]", 200,
                        -participantDisplayExtent, participantDisplayExtent,
                        200, -participantDisplayExtent,
                        participantDisplayExtent);
    TH2F hXY(blastwave::io::kXYHistogramName, "Emission coordinates;x [fm];y [fm]",
             120, -15.0, 15.0, 120, -15.0, 15.0);
    TH2F hPxPy(blastwave::io::kPxPyHistogramName,
               "Transverse momentum map;p_{x} [GeV];p_{y} [GeV]", 120, -3.0,
               3.0, 120, -3.0, 3.0);
    TH1F hPt(blastwave::io::kPtHistogramName,
             "Transverse momentum;p_{T} [GeV];Particles", 120, 0.0, 3.0);
    TH1F hEta(blastwave::io::kEtaHistogramName,
              "Particle pseudorapidity;#eta;Particles", 120, -6.0, 6.0);
    TH1F hPhi(blastwave::io::kPhiHistogramName,
              "Particle azimuth;#phi;Particles", 128, -3.2, 3.2);

    for (int eventId = 0; eventId < config.nEvents; ++eventId) {
      const blastwave::GeneratedEvent event = generator.generateEvent(eventId);

      eventBranches = toEventBranches(event.info);
      eventsTree.Fill();

      hNpart.Fill(eventBranches.nParticipants);
      hEps2.Fill(eventBranches.eps2);
      hPsi2.Fill(eventBranches.psi2);
      hCentrality.Fill(eventBranches.centrality);

      for (const blastwave::ParticipantRecord &participant :
           event.participants) {
        participantBranches = toParticipantBranches(participant);
        participantsTree.Fill();

        hParticipantXY.Fill(participant.x, participant.y);
      }

      for (const blastwave::ParticleRecord &particle : event.particles) {
        particleBranches = toParticleBranches(particle);
        particlesTree.Fill();

        const double pt = std::hypot(particle.px, particle.py);
        hXY.Fill(particle.x, particle.y);
        hPxPy.Fill(particle.px, particle.py);
        hPt.Fill(pt);
        hEta.Fill(computePseudorapidity(particle.px, particle.py, particle.pz));
        hPhi.Fill(std::atan2(particle.py, particle.px));
      }
    }

    TCanvas participantCanvas(blastwave::io::kParticipantXYCanvasName,
                              "Participant nucleons with nucleus outlines", 720,
                              680);
    participantCanvas.SetRightMargin(0.14F);
    hParticipantXY.SetStats(true);
    hParticipantXY.Draw("COLZ");
    participantCanvas.Modified();
    participantCanvas.Update();

    // 人工修复：重复的stats标签问题
    //  if (auto* stats =
    //          dynamic_cast<TPaveStats*>(hParticipantXY.GetListOfFunctions()->FindObject("stats"));
    //      stats != nullptr) {
    //    stats->SetName("stats");
    //    stats->SetX1NDC(0.77);
    //    stats->SetX2NDC(0.98);
    //    stats->SetY1NDC(0.78);
    //    stats->SetY2NDC(0.98);
    //    stats->Draw();
    //  }

    TEllipse nucleusAOutline(-0.5 * config.impactParameter, 0.0,
                             config.woodsSaxonRadius, config.woodsSaxonRadius);
    nucleusAOutline.SetFillStyle(0);
    nucleusAOutline.SetLineColor(4);
    nucleusAOutline.SetLineWidth(2);
    nucleusAOutline.Draw("same");

    TEllipse nucleusBOutline(+0.5 * config.impactParameter, 0.0,
                             config.woodsSaxonRadius, config.woodsSaxonRadius);
    nucleusBOutline.SetFillStyle(0);
    nucleusBOutline.SetLineColor(2);
    nucleusBOutline.SetLineWidth(2);
    nucleusBOutline.Draw("same");

    TLegend participantLegend(0.12, 0.83, 0.40, 0.92);
    participantLegend.SetBorderSize(0);
    participantLegend.SetFillStyle(0);
    participantLegend.AddEntry(&nucleusAOutline, "Nucleus A outline", "l");
    participantLegend.AddEntry(&nucleusBOutline, "Nucleus B outline", "l");
    participantLegend.Draw();
    participantCanvas.Modified();
    participantCanvas.Update();

    outputFile.cd();
    eventsTree.Write();
    participantsTree.Write();
    particlesTree.Write();
    hNpart.Write();
    hEps2.Write();
    hPsi2.Write();
    hCentrality.Write();
    hParticipantXY.Write();
    hXY.Write();
    hPxPy.Write();
    hPt.Write();
    hEta.Write();
    hPhi.Write();
    participantCanvas.Write();
    outputFile.Close();

    std::cout << "Wrote " << config.nEvents << " events to " << outputPath
              << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "generate_blastwave_events failed: " << error.what() << '\n';
    return 1;
  }
}
