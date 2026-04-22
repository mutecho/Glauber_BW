#include <TFile.h>
#include <TH1F.h>
#include <TH2.h>
#include <TH2F.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>

#include "blastwave/BlastWaveGenerator.h"
#include "blastwave/PhysicsUtils.h"
#include "blastwave/io/OutputPathUtils.h"
#include "blastwave/io/RootOutputSchema.h"

namespace {

  void printUsage(const char *programName) {
    std::cout << "Usage: " << programName << " --input <file.root> [--output <qa.root>] "
              << "[--expect-nevents <int>]\n";
  }

  bool isFinite(double value) {
    return std::isfinite(value);
  }

  bool nearlyEqual(double lhs, double rhs, double relativeTolerance) {
    const double scale = std::max({1.0, std::abs(lhs), std::abs(rhs)});
    return std::abs(lhs - rhs) <= relativeTolerance * scale;
  }

  double computeExpectedCentrality(double impactParameter) {
    const double woodsSaxonRadius = blastwave::BlastWaveConfig{}.woodsSaxonRadius;
    return blastwave::computeCentralityPercent(impactParameter, woodsSaxonRadius);
  }

}  // namespace

int main(int argc, char **argv) {
  try {
    std::string inputPath;
    std::string outputPath = "qa_validation.root";
    int expectedEvents = -1;

    for (int iArg = 1; iArg < argc; ++iArg) {
      const std::string option = argv[iArg];
      if (option == "--help") {
        printUsage(argv[0]);
        return 0;
      }
      if (iArg + 1 >= argc) {
        throw std::invalid_argument("Missing value for " + option);
      }
      const std::string value = argv[++iArg];
      if (option == "--input") {
        inputPath = value;
      } else if (option == "--output") {
        outputPath = value;
      } else if (option == "--expect-nevents") {
        expectedEvents = std::stoi(value);
      } else {
        throw std::invalid_argument("Unknown option: " + option);
      }
    }

    if (inputPath.empty()) {
      throw std::invalid_argument("--input is required.");
    }

    TFile inputFile(inputPath.c_str(), "READ");
    if (inputFile.IsZombie()) {
      throw std::runtime_error("Failed to open input ROOT file: " + inputPath);
    }

    auto *eventsTree = dynamic_cast<TTree *>(inputFile.Get(blastwave::io::kEventsTreeName));
    auto *participantsTree = dynamic_cast<TTree *>(inputFile.Get(blastwave::io::kParticipantsTreeName));
    auto *particlesTree = dynamic_cast<TTree *>(inputFile.Get(blastwave::io::kParticlesTreeName));
    if (eventsTree == nullptr || participantsTree == nullptr || particlesTree == nullptr) {
      throw std::runtime_error("Required trees 'events', 'participants', or 'particles' are missing.");
    }

    const char *requiredObjects[] = {blastwave::io::kNpartHistogramName,
                                     blastwave::io::kEps2HistogramName,
                                     blastwave::io::kPsi2HistogramName,
                                     blastwave::io::kV2HistogramName,
                                     blastwave::io::kCentralityHistogramName,
                                     blastwave::io::kParticipantXYHistogramName,
                                     blastwave::io::kParticipantXYCanvasName,
                                     blastwave::io::kXYHistogramName,
                                     blastwave::io::kPxPyHistogramName,
                                     blastwave::io::kPtHistogramName,
                                     blastwave::io::kEtaHistogramName,
                                     blastwave::io::kPhiHistogramName};
    for (const char *objectName : requiredObjects) {
      if (inputFile.Get(objectName) == nullptr) {
        throw std::runtime_error(std::string("Missing required QA object: ") + objectName);
      }
    }

    blastwave::io::EventBranches eventBranches;
    blastwave::io::ParticipantBranches participantBranches;
    blastwave::io::ParticleBranches particleBranches;

    auto *participantXYInput = dynamic_cast<TH2 *>(inputFile.Get(blastwave::io::kParticipantXYHistogramName));
    if (participantXYInput == nullptr) {
      throw std::runtime_error("Input object 'participant_x-y' is not a TH2.");
    }
    auto *centralityInput = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kCentralityHistogramName));
    if (centralityInput == nullptr) {
      throw std::runtime_error("Input object 'cent' is not a TH1.");
    }
    auto *v2Input = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kV2HistogramName));
    if (v2Input == nullptr) {
      throw std::runtime_error("Input object 'v2' is not a TH1.");
    }

    // Optional flow-ellipse debug payload uses "exists then validate" behavior.
    auto *flowEllipseDebugTree = dynamic_cast<TTree *>(inputFile.Get(blastwave::io::kFlowEllipseDebugTreeName));
    if (inputFile.Get(blastwave::io::kFlowEllipseDebugTreeName) != nullptr && flowEllipseDebugTree == nullptr) {
      throw std::runtime_error("Input object 'flow_ellipse_debug' is not a TTree.");
    }
    auto *flowEllipseParticipantNormXYInput = dynamic_cast<TH2 *>(inputFile.Get(blastwave::io::kFlowEllipseParticipantNormXYHistogramName));
    if (inputFile.Get(blastwave::io::kFlowEllipseParticipantNormXYHistogramName) != nullptr && flowEllipseParticipantNormXYInput == nullptr) {
      throw std::runtime_error("Input object 'flow_ellipse_participant_norm_x-y' is not a TH2.");
    }
    if (flowEllipseDebugTree != nullptr && flowEllipseParticipantNormXYInput == nullptr) {
      throw std::runtime_error("Input tree 'flow_ellipse_debug' exists without companion 'flow_ellipse_participant_norm_x-y' histogram.");
    }
    if (flowEllipseParticipantNormXYInput != nullptr && flowEllipseDebugTree == nullptr) {
      throw std::runtime_error("Input object 'flow_ellipse_participant_norm_x-y' exists without companion 'flow_ellipse_debug' tree.");
    }

    blastwave::io::bindEventBranches(*eventsTree, eventBranches);
    blastwave::io::bindParticipantBranches(*participantsTree, participantBranches);
    blastwave::io::bindParticleBranches(*particlesTree, particleBranches);

    TH2F hParticipantXY("qa_participant_x-y",
                        "QA participant nucleons;x [fm];y [fm]",
                        participantXYInput->GetNbinsX(),
                        participantXYInput->GetXaxis()->GetXmin(),
                        participantXYInput->GetXaxis()->GetXmax(),
                        participantXYInput->GetNbinsY(),
                        participantXYInput->GetYaxis()->GetXmin(),
                        participantXYInput->GetYaxis()->GetXmax());
    TH2F hXY("qa_x-y", "QA emission coordinates;x [fm];y [fm]", 120, -15.0, 15.0, 120, -15.0, 15.0);
    TH2F hPxPy("qa_px-py", "QA transverse momentum map;p_{x} [GeV];p_{y} [GeV]", 120, -3.0, 3.0, 120, -3.0, 3.0);
    TH1F hPsi2("qa_psi2", "QA participant-plane angle;#Psi_{2} [rad];Events", 128, -1.7, 1.7);
    TH1F hV2("qa_v2", "QA event-by-event final-state v_{2};v_{2};Events", 120, 0.0, 1.0);
    TH1F hPt("qa_pT", "QA transverse momentum;p_{T} [GeV];Particles", 120, 0.0, 3.0);
    TH1F hEta("qa_eta", "QA pseudorapidity;#eta;Particles", 120, -6.0, 6.0);
    TH1F hPhi("qa_phi", "QA azimuth;#phi;Particles", 128, -3.2, 3.2);

    std::unordered_map<int, int> participantCountsByEvent;
    std::unordered_map<int, int> particleCountsByEvent;
    std::unordered_map<int, bool> eventIdsSeen;
    std::unordered_map<int, bool> flowEllipseValidByEvent;
    std::unordered_map<int, double> q2xByEvent;
    std::unordered_map<int, double> q2yByEvent;
    double maxMassShellDeviation = 0.0;
    double maxAbsEtaS = 0.0;
    double maxEnergy = 0.0;

    for (Long64_t iEntry = 0; iEntry < participantsTree->GetEntries(); ++iEntry) {
      participantsTree->GetEntry(iEntry);

      const double fields[] = {participantBranches.x, participantBranches.y, participantBranches.z};
      for (double field : fields) {
        if (!isFinite(field)) {
          throw std::runtime_error("Detected NaN/Inf in participants tree.");
        }
      }
      if (participantBranches.nucleusId != 0 && participantBranches.nucleusId != 1) {
        throw std::runtime_error("participants.nucleus_id must be 0 or 1.");
      }

      ++participantCountsByEvent[participantBranches.eventId];
      hParticipantXY.Fill(participantBranches.x, participantBranches.y);
    }

    for (Long64_t iEntry = 0; iEntry < particlesTree->GetEntries(); ++iEntry) {
      particlesTree->GetEntry(iEntry);

      const double fields[] = {particleBranches.mass,
                               particleBranches.x,
                               particleBranches.y,
                               particleBranches.z,
                               particleBranches.t,
                               particleBranches.px,
                               particleBranches.py,
                               particleBranches.pz,
                               particleBranches.energy,
                               particleBranches.etaS,
                               particleBranches.sourceX,
                               particleBranches.sourceY};
      for (double field : fields) {
        if (!isFinite(field)) {
          throw std::runtime_error("Detected NaN/Inf in particle tree.");
        }
      }

      const double massShellDeviation =
          std::abs(particleBranches.energy * particleBranches.energy
                   - (particleBranches.px * particleBranches.px + particleBranches.py * particleBranches.py + particleBranches.pz * particleBranches.pz)
                   - particleBranches.mass * particleBranches.mass);
      maxMassShellDeviation = std::max(maxMassShellDeviation, massShellDeviation);
      maxAbsEtaS = std::max(maxAbsEtaS, std::abs(static_cast<double>(particleBranches.etaS)));
      maxEnergy = std::max(maxEnergy, static_cast<double>(particleBranches.energy));

      ++particleCountsByEvent[particleBranches.eventId];
      const double phi = blastwave::computeAzimuth(particleBranches.px, particleBranches.py);
      q2xByEvent[particleBranches.eventId] += std::cos(2.0 * phi);
      q2yByEvent[particleBranches.eventId] += std::sin(2.0 * phi);
      hXY.Fill(particleBranches.x, particleBranches.y);
      hPxPy.Fill(particleBranches.px, particleBranches.py);
      hPt.Fill(std::hypot(particleBranches.px, particleBranches.py));
      hEta.Fill(blastwave::computePseudorapidity(particleBranches.px, particleBranches.py, particleBranches.pz));
      hPhi.Fill(phi);
    }

    if (maxMassShellDeviation > 1.0e-4) {
      throw std::runtime_error("Mass-shell validation exceeded 1e-4 GeV^2. max deviation = " + std::to_string(maxMassShellDeviation)
                               + ", max |eta_s| = " + std::to_string(maxAbsEtaS) + ", max E = " + std::to_string(maxEnergy));
    }

    double meanNpart = 0.0;
    double meanEps2 = 0.0;
    double meanV2 = 0.0;
    bool sawFirstCentrality = false;
    double firstCentrality = 0.0;
    int previousEventId = -1;

    for (Long64_t iEntry = 0; iEntry < eventsTree->GetEntries(); ++iEntry) {
      eventsTree->GetEntry(iEntry);

      if (previousEventId >= 0 && eventBranches.eventId != previousEventId + 1) {
        throw std::runtime_error("event_id sequence is not continuous.");
      }
      previousEventId = eventBranches.eventId;

      const int observedParticleCount = particleCountsByEvent.count(eventBranches.eventId) > 0 ? particleCountsByEvent[eventBranches.eventId] : 0;
      const int observedParticipantCount = participantCountsByEvent.count(eventBranches.eventId) > 0 ? participantCountsByEvent[eventBranches.eventId] : 0;
      if (observedParticipantCount != eventBranches.nParticipants) {
        throw std::runtime_error("Npart does not match participant multiplicity for an event.");
      }
      if (observedParticleCount != eventBranches.nCharged) {
        throw std::runtime_error("Nch does not match particle multiplicity for an event.");
      }
      const double expectedV2 = blastwave::computeSecondHarmonicEventV2(q2xByEvent[eventBranches.eventId], q2yByEvent[eventBranches.eventId], observedParticleCount);
      if (!isFinite(eventBranches.v2) || eventBranches.v2 < 0.0 || eventBranches.v2 > 1.0) {
        throw std::runtime_error("v2 must stay within [0, 1].");
      }
      if (!nearlyEqual(eventBranches.v2, expectedV2, 1.0e-9)) {
        throw std::runtime_error("events.v2 does not match the particle-level second-harmonic Q-vector.");
      }
      if (!isFinite(eventBranches.centrality) || eventBranches.centrality < 0.0 || eventBranches.centrality > 100.0) {
        throw std::runtime_error("centrality must stay within [0, 100].");
      }

      const double expectedCentrality = computeExpectedCentrality(eventBranches.impactParameter);
      if (std::abs(eventBranches.centrality - expectedCentrality) > 1.0e-9) {
        throw std::runtime_error("centrality does not match the expected impact-parameter mapping.");
      }
      if (!sawFirstCentrality) {
        firstCentrality = eventBranches.centrality;
        sawFirstCentrality = true;
      } else if (std::abs(eventBranches.centrality - firstCentrality) > 1.0e-9) {
        throw std::runtime_error("centrality should remain constant for a fixed-impact-parameter run.");
      }

      meanNpart += eventBranches.nParticipants;
      meanEps2 += eventBranches.eps2;
      meanV2 += eventBranches.v2;
      hPsi2.Fill(eventBranches.psi2);
      hV2.Fill(eventBranches.v2);
      eventIdsSeen[eventBranches.eventId] = true;
    }

    if (expectedEvents >= 0 && eventsTree->GetEntries() != expectedEvents) {
      throw std::runtime_error("events tree entry count does not match --expect-nevents.");
    }

    if (flowEllipseDebugTree != nullptr) {
      if (flowEllipseDebugTree->GetEntries() != eventsTree->GetEntries()) {
        throw std::runtime_error("flow_ellipse_debug entry count must match events tree entry count.");
      }

      blastwave::io::FlowEllipseDebugBranches flowEllipseBranches;
      blastwave::io::bindFlowEllipseDebugBranches(*flowEllipseDebugTree, flowEllipseBranches);

      // Enforce debug-shape invariants when the optional debug tree exists.
      for (Long64_t iEntry = 0; iEntry < flowEllipseDebugTree->GetEntries(); ++iEntry) {
        flowEllipseDebugTree->GetEntry(iEntry);

        const double fields[] = {flowEllipseBranches.centerX,
                                 flowEllipseBranches.centerY,
                                 flowEllipseBranches.sigmaX2,
                                 flowEllipseBranches.sigmaY2,
                                 flowEllipseBranches.sigmaXY,
                                 flowEllipseBranches.lambdaMajor,
                                 flowEllipseBranches.lambdaMinor,
                                 flowEllipseBranches.radiusMajor,
                                 flowEllipseBranches.radiusMinor,
                                 flowEllipseBranches.majorAxisX,
                                 flowEllipseBranches.majorAxisY,
                                 flowEllipseBranches.minorAxisX,
                                 flowEllipseBranches.minorAxisY,
                                 flowEllipseBranches.eps2,
                                 flowEllipseBranches.psi2};
        for (double field : fields) {
          if (!isFinite(field)) {
            throw std::runtime_error("Detected NaN/Inf in flow_ellipse_debug tree.");
          }
        }
        if (flowEllipseBranches.lambdaMajor < flowEllipseBranches.lambdaMinor || flowEllipseBranches.lambdaMinor < 0.0) {
          throw std::runtime_error("flow_ellipse_debug must satisfy lambdaMajor >= lambdaMinor >= 0.");
        }
        if (!nearlyEqual(flowEllipseBranches.radiusMajor * flowEllipseBranches.radiusMajor, flowEllipseBranches.lambdaMajor, 1.0e-9)
            || !nearlyEqual(flowEllipseBranches.radiusMinor * flowEllipseBranches.radiusMinor, flowEllipseBranches.lambdaMinor, 1.0e-9)) {
          throw std::runtime_error("flow_ellipse_debug radii must satisfy R^2 == lambda for both axes.");
        }

        const double majorNorm2 = flowEllipseBranches.majorAxisX * flowEllipseBranches.majorAxisX + flowEllipseBranches.majorAxisY * flowEllipseBranches.majorAxisY;
        const double minorNorm2 = flowEllipseBranches.minorAxisX * flowEllipseBranches.minorAxisX + flowEllipseBranches.minorAxisY * flowEllipseBranches.minorAxisY;
        const double axisDot = flowEllipseBranches.majorAxisX * flowEllipseBranches.minorAxisX + flowEllipseBranches.majorAxisY * flowEllipseBranches.minorAxisY;
        if (!nearlyEqual(majorNorm2, 1.0, 1.0e-9) || !nearlyEqual(minorNorm2, 1.0, 1.0e-9) || !nearlyEqual(axisDot, 0.0, 1.0e-9)) {
          throw std::runtime_error("flow_ellipse_debug axes must be orthonormal.");
        }
        if (flowEllipseBranches.valid && (flowEllipseBranches.radiusMajor <= 0.0 || flowEllipseBranches.radiusMinor <= 0.0)) {
          throw std::runtime_error("flow_ellipse_debug valid events must have positive semi-axes.");
        }

        const bool inserted = flowEllipseValidByEvent.emplace(flowEllipseBranches.eventId, static_cast<bool>(flowEllipseBranches.valid)).second;
        if (!inserted) {
          throw std::runtime_error("flow_ellipse_debug event_id must be unique.");
        }
      }
      for (const auto &[eventId, _] : eventIdsSeen) {
        if (flowEllipseValidByEvent.count(eventId) == 0U) {
          throw std::runtime_error("flow_ellipse_debug is missing an event_id present in the events tree.");
        }
      }
    }

    if (flowEllipseParticipantNormXYInput != nullptr) {
      Long64_t expectedValidParticipantFillCount = 0;
      for (const auto &[eventId, ellipseValid] : flowEllipseValidByEvent) {
        if (!ellipseValid) {
          continue;
        }
        const auto countIt = participantCountsByEvent.find(eventId);
        if (countIt != participantCountsByEvent.end()) {
          expectedValidParticipantFillCount += static_cast<Long64_t>(countIt->second);
        }
      }
      if (flowEllipseParticipantNormXYInput->GetEntries() < static_cast<double>(expectedValidParticipantFillCount)) {
        throw std::runtime_error("flow_ellipse_participant_norm_x-y entries are smaller than valid-event participant fill expectation.");
      }
    }

    const double eventCount = static_cast<double>(eventsTree->GetEntries());
    if (eventCount > 0.0) {
      meanNpart /= eventCount;
      meanEps2 /= eventCount;
      meanV2 /= eventCount;
    }
    if (std::abs(centralityInput->GetEntries() - eventCount) > 1.0e-9) {
      throw std::runtime_error("cent histogram entry count does not match events tree.");
    }
    if (std::abs(v2Input->GetEntries() - eventCount) > 1.0e-9) {
      throw std::runtime_error("v2 histogram entry count does not match events tree.");
    }
    if (eventCount > 0.0 && !nearlyEqual(v2Input->GetMean(), meanV2, 1.0e-9)) {
      throw std::runtime_error("v2 histogram mean does not match the events.v2 mean.");
    }

    // Make QA output behavior match the generator: create missing parent
    // directories instead of failing inside ROOT file construction.
    blastwave::io::ensureOutputDirectoryExists(outputPath, std::cout);
    TFile qaFile(outputPath.c_str(), "RECREATE");
    if (qaFile.IsZombie()) {
      throw std::runtime_error("Failed to create QA ROOT file: " + outputPath);
    }
    hParticipantXY.Write();
    hXY.Write();
    hPxPy.Write();
    hPsi2.Write();
    hV2.Write();
    hPt.Write();
    hEta.Write();
    hPhi.Write();
    qaFile.Close();

    std::cout << "validation_passed"
              << " events=" << eventsTree->GetEntries() << " particles=" << particlesTree->GetEntries() << " mean_Npart=" << meanNpart << " mean_eps2=" << meanEps2
              << " mean_v2=" << meanV2 << " max_abs_eta_s=" << maxAbsEtaS << " max_E=" << maxEnergy << " max_mass_shell_deviation=" << maxMassShellDeviation << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "qa_blastwave_output failed: " << error.what() << '\n';
    return 1;
  }
}
