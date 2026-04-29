#include <TCanvas.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2.h>
#include <TH2F.h>
#include <TTree.h>
#include <TVectorD.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "blastwave/BlastWaveGenerator.h"
#include "blastwave/PhysicsUtils.h"
#include "blastwave/V2PtCumulant.h"
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

  std::string toUpperCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) { return static_cast<char>(std::toupper(character)); });
    return value;
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
                                     blastwave::io::kEps2FreezeoutHistogramName,
                                     blastwave::io::kPsi2HistogramName,
                                     blastwave::io::kPsi2FreezeoutHistogramName,
                                     blastwave::io::kChi2HistogramName,
                                     blastwave::io::kR2InitialHistogramName,
                                     blastwave::io::kR2FinalHistogramName,
                                     blastwave::io::kR2RatioHistogramName,
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
    auto *eps2FreezeoutInput = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kEps2FreezeoutHistogramName));
    if (eps2FreezeoutInput == nullptr) {
      throw std::runtime_error("Input object 'eps2_f' is not a TH1.");
    }
    auto *psi2FreezeoutInput = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kPsi2FreezeoutHistogramName));
    if (psi2FreezeoutInput == nullptr) {
      throw std::runtime_error("Input object 'psi2_f' is not a TH1.");
    }
    auto *chi2Input = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kChi2HistogramName));
    if (chi2Input == nullptr) {
      throw std::runtime_error("Input object 'chi2' is not a TH1.");
    }
    auto *r2InitialInput = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kR2InitialHistogramName));
    if (r2InitialInput == nullptr) {
      throw std::runtime_error("Input object 'r2_0' is not a TH1.");
    }
    auto *r2FinalInput = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kR2FinalHistogramName));
    if (r2FinalInput == nullptr) {
      throw std::runtime_error("Input object 'r2_f' is not a TH1.");
    }
    auto *r2RatioInput = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kR2RatioHistogramName));
    if (r2RatioInput == nullptr) {
      throw std::runtime_error("Input object 'r2_ratio' is not a TH1.");
    }
    auto *v2Input = dynamic_cast<TH1 *>(inputFile.Get(blastwave::io::kV2HistogramName));
    if (v2Input == nullptr) {
      throw std::runtime_error("Input object 'v2' is not a TH1.");
    }

    auto *v2PtEdgesObject = inputFile.Get(blastwave::io::kV2PtEdgesObjectName);
    auto *v2PtHistogramObject = inputFile.Get(blastwave::io::kV2PtHistogramName);
    auto *v2PtCanvasObject = inputFile.Get(blastwave::io::kV2PtCanvasName);
    const bool hasV2PtEdges = v2PtEdgesObject != nullptr;
    const bool hasV2PtHistogram = v2PtHistogramObject != nullptr;
    const bool hasV2PtCanvas = v2PtCanvasObject != nullptr;
    const bool hasAnyV2PtPayload = hasV2PtHistogram || hasV2PtCanvas;
    if (hasAnyV2PtPayload && (!hasV2PtHistogram || !hasV2PtCanvas || !hasV2PtEdges)) {
      throw std::runtime_error("v2pt analysis payload must contain all objects: v2_2_pt_edges, v2_2_pt, v2_2_pt_canvas.");
    }
    if (!hasAnyV2PtPayload && (hasV2PtHistogram || hasV2PtCanvas)) {
      throw std::runtime_error("Invalid v2pt object state: analysis payload cannot be partially present.");
    }

    std::vector<double> v2PtEdges;
    if (hasV2PtEdges) {
      auto *v2PtEdgesVector = dynamic_cast<TVectorD *>(v2PtEdgesObject);
      if (v2PtEdgesVector == nullptr) {
        throw std::runtime_error("Input object 'v2_2_pt_edges' is not a TVectorD.");
      }
      v2PtEdges.reserve(static_cast<std::size_t>(v2PtEdgesVector->GetNoElements()));
      for (int iEdge = 0; iEdge < v2PtEdgesVector->GetNoElements(); ++iEdge) {
        const double edge = (*v2PtEdgesVector)[iEdge];
        if (!isFinite(edge) || edge < 0.0) {
          throw std::runtime_error("v2_2_pt_edges must be finite and non-negative.");
        }
        if (!v2PtEdges.empty() && !(edge > v2PtEdges.back())) {
          throw std::runtime_error("v2_2_pt_edges must be strictly increasing.");
        }
        v2PtEdges.push_back(edge);
      }
      if (v2PtEdges.size() < 2U) {
        throw std::runtime_error("v2_2_pt_edges must contain at least two entries.");
      }
    }

    auto *v2PtHistogramInput = hasV2PtHistogram ? dynamic_cast<TH1 *>(v2PtHistogramObject) : nullptr;
    if (hasV2PtHistogram && v2PtHistogramInput == nullptr) {
      throw std::runtime_error("Input object 'v2_2_pt' is not a TH1.");
    }
    auto *v2PtCanvasInput = hasV2PtCanvas ? dynamic_cast<TCanvas *>(v2PtCanvasObject) : nullptr;
    if (hasV2PtCanvas && v2PtCanvasInput == nullptr) {
      throw std::runtime_error("Input object 'v2_2_pt_canvas' is not a TCanvas.");
    }
    (void)v2PtCanvasInput;

    // Optional flow-ellipse debug payload uses "exists then validate" behavior.
    auto *flowEllipseDebugTree = dynamic_cast<TTree *>(inputFile.Get(blastwave::io::kFlowEllipseDebugTreeName));
    if (inputFile.Get(blastwave::io::kFlowEllipseDebugTreeName) != nullptr && flowEllipseDebugTree == nullptr) {
      throw std::runtime_error("Input object 'flow_ellipse_debug' is not a TTree.");
    }
    auto *flowEllipseParticipantNormXYInput = dynamic_cast<TH2 *>(inputFile.Get(blastwave::io::kFlowEllipseParticipantNormXYHistogramName));
    if (inputFile.Get(blastwave::io::kFlowEllipseParticipantNormXYHistogramName) != nullptr && flowEllipseParticipantNormXYInput == nullptr) {
      throw std::runtime_error("Input object 'flow_ellipse_participant_norm_x-y' is not a TH2.");
    }
    auto *densityNormalEventDensityInput = dynamic_cast<TH2 *>(inputFile.Get(blastwave::io::kDensityNormalEventDensityHistogramName));
    if (inputFile.Get(blastwave::io::kDensityNormalEventDensityHistogramName) != nullptr && densityNormalEventDensityInput == nullptr) {
      throw std::runtime_error("Input object 'density_normal_event_density_x-y' is not a TH2.");
    }
    auto *gradientS0Input = dynamic_cast<TH2 *>(inputFile.Get(blastwave::io::kGradientS0HistogramName));
    auto *gradientSEmInput = dynamic_cast<TH2 *>(inputFile.Get(blastwave::io::kGradientSEmHistogramName));
    auto *gradientSDynInput = dynamic_cast<TH2 *>(inputFile.Get(blastwave::io::kGradientSDynHistogramName));
    auto *gradientSFInput = dynamic_cast<TH2 *>(inputFile.Get(blastwave::io::kGradientSFHistogramName));
    if (inputFile.Get(blastwave::io::kGradientS0HistogramName) != nullptr && gradientS0Input == nullptr) {
      throw std::runtime_error("Input object 'gradient_s0_x-y' is not a TH2.");
    }
    if (inputFile.Get(blastwave::io::kGradientSEmHistogramName) != nullptr && gradientSEmInput == nullptr) {
      throw std::runtime_error("Input object 'gradient_s_em_x-y' is not a TH2.");
    }
    if (inputFile.Get(blastwave::io::kGradientSDynHistogramName) != nullptr && gradientSDynInput == nullptr) {
      throw std::runtime_error("Input object 'gradient_s_dyn_x-y' is not a TH2.");
    }
    if (inputFile.Get(blastwave::io::kGradientSFHistogramName) != nullptr && gradientSFInput == nullptr) {
      throw std::runtime_error("Input object 'gradient_s_f_x-y' is not a TH2.");
    }
    const bool hasAnyGradientDebugHistogram = gradientS0Input != nullptr || gradientSEmInput != nullptr || gradientSDynInput != nullptr || gradientSFInput != nullptr;
    if (hasAnyGradientDebugHistogram && (gradientS0Input == nullptr || gradientSEmInput == nullptr || gradientSDynInput == nullptr || gradientSFInput == nullptr)) {
      throw std::runtime_error("Gradient debug payload must contain all four histograms: gradient_s0_x-y, gradient_s_em_x-y, gradient_s_dyn_x-y, gradient_s_f_x-y.");
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
    std::unordered_map<int, double> q2WeightByEvent;
    std::unordered_map<int, double> x0SumByEvent;
    std::unordered_map<int, double> y0SumByEvent;
    std::unordered_map<int, double> r2InitialSumByEvent;
    std::unordered_map<int, double> xSumByEvent;
    std::unordered_map<int, double> ySumByEvent;
    std::unordered_map<int, double> r2FinalSumByEvent;
    std::unordered_map<int, std::vector<blastwave::V2PtTrack>> v2TracksByEvent;
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
                               particleBranches.sourceY,
                               particleBranches.x0,
                               particleBranches.y0,
                               particleBranches.emissionWeight};
      for (double field : fields) {
        if (!isFinite(field)) {
          throw std::runtime_error("Detected NaN/Inf in particle tree.");
        }
      }
      if (particleBranches.emissionWeight < 0.0) {
        throw std::runtime_error("particle.emission_weight must be non-negative.");
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
      q2xByEvent[particleBranches.eventId] += particleBranches.emissionWeight * std::cos(2.0 * phi);
      q2yByEvent[particleBranches.eventId] += particleBranches.emissionWeight * std::sin(2.0 * phi);
      q2WeightByEvent[particleBranches.eventId] += particleBranches.emissionWeight;
      x0SumByEvent[particleBranches.eventId] += particleBranches.x0;
      y0SumByEvent[particleBranches.eventId] += particleBranches.y0;
      r2InitialSumByEvent[particleBranches.eventId] += particleBranches.x0 * particleBranches.x0 + particleBranches.y0 * particleBranches.y0;
      xSumByEvent[particleBranches.eventId] += particleBranches.x;
      ySumByEvent[particleBranches.eventId] += particleBranches.y;
      r2FinalSumByEvent[particleBranches.eventId] += particleBranches.x * particleBranches.x + particleBranches.y * particleBranches.y;
      hXY.Fill(particleBranches.x, particleBranches.y);
      hPxPy.Fill(particleBranches.px, particleBranches.py);
      hPt.Fill(std::hypot(particleBranches.px, particleBranches.py));
      hEta.Fill(blastwave::computePseudorapidity(particleBranches.px, particleBranches.py, particleBranches.pz));
      hPhi.Fill(phi);
      v2TracksByEvent[particleBranches.eventId].push_back({particleBranches.px, particleBranches.py});
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
      const double expectedV2 =
          q2WeightByEvent[eventBranches.eventId] > 0.0
              ? std::hypot(q2xByEvent[eventBranches.eventId], q2yByEvent[eventBranches.eventId]) / q2WeightByEvent[eventBranches.eventId]
              : 0.0;
      if (!isFinite(eventBranches.v2) || eventBranches.v2 < 0.0 || eventBranches.v2 > 1.0) {
        throw std::runtime_error("v2 must stay within [0, 1].");
      }
      if (!nearlyEqual(eventBranches.v2, expectedV2, 1.0e-9)) {
        throw std::runtime_error("events.v2 does not match the particle-level second-harmonic Q-vector.");
      }
      if (!isFinite(eventBranches.centrality) || eventBranches.centrality < 0.0 || eventBranches.centrality > 100.0) {
        throw std::runtime_error("centrality must stay within [0, 100].");
      }
      if (!isFinite(eventBranches.eps2Freezeout) || eventBranches.eps2Freezeout < 0.0 || eventBranches.eps2Freezeout > 1.0) {
        throw std::runtime_error("eps2_f must stay within [0, 1].");
      }
      if (!isFinite(eventBranches.psi2Freezeout)) {
        throw std::runtime_error("psi2_f must be finite.");
      }
      if (!isFinite(eventBranches.chi2) || eventBranches.chi2 < 0.0) {
        throw std::runtime_error("chi2 must be finite and non-negative.");
      }
      const double expectedChi2 = eventBranches.eps2 > 1.0e-12 ? eventBranches.eps2Freezeout / eventBranches.eps2 : 0.0;
      if (!nearlyEqual(eventBranches.chi2, expectedChi2, 1.0e-9)) {
        throw std::runtime_error("chi2 must equal eps2_f / eps2 when eps2 > 1e-12, otherwise 0.");
      }
      if (!isFinite(eventBranches.r2Initial) || eventBranches.r2Initial < 0.0) {
        throw std::runtime_error("r2_0 must be finite and non-negative.");
      }
      if (!isFinite(eventBranches.r2Final) || eventBranches.r2Final < 0.0) {
        throw std::runtime_error("r2_f must be finite and non-negative.");
      }
      if (!isFinite(eventBranches.r2Ratio) || eventBranches.r2Ratio < 0.0) {
        throw std::runtime_error("r2_ratio must be finite and non-negative.");
      }
      double observedR2Initial = 0.0;
      double observedR2Final = 0.0;
      if (observedParticleCount > 0) {
        const double inverseCount = 1.0 / static_cast<double>(observedParticleCount);
        const double meanX0 = x0SumByEvent[eventBranches.eventId] * inverseCount;
        const double meanY0 = y0SumByEvent[eventBranches.eventId] * inverseCount;
        const double meanX = xSumByEvent[eventBranches.eventId] * inverseCount;
        const double meanY = ySumByEvent[eventBranches.eventId] * inverseCount;
        observedR2Initial = std::max(0.0, r2InitialSumByEvent[eventBranches.eventId] * inverseCount - meanX0 * meanX0 - meanY0 * meanY0);
        observedR2Final = std::max(0.0, r2FinalSumByEvent[eventBranches.eventId] * inverseCount - meanX * meanX - meanY * meanY);
      }
      const double observedR2Ratio = observedR2Initial > 1.0e-12 ? observedR2Final / observedR2Initial : 0.0;
      if (!nearlyEqual(eventBranches.r2Initial, observedR2Initial, 1.0e-9)) {
        throw std::runtime_error("events.r2_0 does not match particle-level x0/y0 second moment.");
      }
      if (!nearlyEqual(eventBranches.r2Final, observedR2Final, 1.0e-9)) {
        throw std::runtime_error("events.r2_f does not match particle-level x/y second moment.");
      }
      if (!nearlyEqual(eventBranches.r2Ratio, observedR2Ratio, 1.0e-9)) {
        throw std::runtime_error("events.r2_ratio does not match particle-level r2_f/r2_0.");
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

    if (densityNormalEventDensityInput != nullptr) {
      const std::string drawOption = toUpperCopy(densityNormalEventDensityInput->GetOption());
      if (drawOption.find("LEGO") == std::string::npos && drawOption.find("SURF") == std::string::npos) {
        throw std::runtime_error("density_normal_event_density_x-y must default to a 3D ROOT draw option.");
      }

      bool sawPositiveDensityBin = false;
      for (int iBinX = 1; iBinX <= densityNormalEventDensityInput->GetNbinsX(); ++iBinX) {
        for (int iBinY = 1; iBinY <= densityNormalEventDensityInput->GetNbinsY(); ++iBinY) {
          const double binContent = densityNormalEventDensityInput->GetBinContent(iBinX, iBinY);
          if (!isFinite(binContent)) {
            throw std::runtime_error("density_normal_event_density_x-y contains NaN/Inf bin content.");
          }
          if (binContent < 0.0) {
            throw std::runtime_error("density_normal_event_density_x-y contains a negative density bin.");
          }
          sawPositiveDensityBin = sawPositiveDensityBin || binContent > 0.0;
        }
      }
      if (!sawPositiveDensityBin) {
        throw std::runtime_error("density_normal_event_density_x-y must contain at least one positive density bin.");
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
    if (std::abs(eps2FreezeoutInput->GetEntries() - eventCount) > 1.0e-9) {
      throw std::runtime_error("eps2_f histogram entry count does not match events tree.");
    }
    if (std::abs(psi2FreezeoutInput->GetEntries() - eventCount) > 1.0e-9) {
      throw std::runtime_error("psi2_f histogram entry count does not match events tree.");
    }
    if (std::abs(chi2Input->GetEntries() - eventCount) > 1.0e-9) {
      throw std::runtime_error("chi2 histogram entry count does not match events tree.");
    }
    if (std::abs(r2InitialInput->GetEntries() - eventCount) > 1.0e-9) {
      throw std::runtime_error("r2_0 histogram entry count does not match events tree.");
    }
    if (std::abs(r2FinalInput->GetEntries() - eventCount) > 1.0e-9) {
      throw std::runtime_error("r2_f histogram entry count does not match events tree.");
    }
    if (std::abs(r2RatioInput->GetEntries() - eventCount) > 1.0e-9) {
      throw std::runtime_error("r2_ratio histogram entry count does not match events tree.");
    }
    if (eventCount > 0.0 && !nearlyEqual(v2Input->GetMean(), meanV2, 1.0e-9)) {
      throw std::runtime_error("v2 histogram mean does not match the events.v2 mean.");
    }

    if (hasV2PtHistogram) {
      const int expectedBins = static_cast<int>(v2PtEdges.size()) - 1;
      if (v2PtHistogramInput->GetNbinsX() != expectedBins) {
        throw std::runtime_error("v2_2_pt bin count does not match v2_2_pt_edges.");
      }
      for (int iBin = 1; iBin <= expectedBins; ++iBin) {
        const double expectedLowEdge = v2PtEdges[static_cast<std::size_t>(iBin - 1)];
        const double expectedUpperEdge = v2PtEdges[static_cast<std::size_t>(iBin)];
        const double actualLowEdge = v2PtHistogramInput->GetXaxis()->GetBinLowEdge(iBin);
        const double actualUpperEdge = v2PtHistogramInput->GetXaxis()->GetBinUpEdge(iBin);
        if (!nearlyEqual(actualLowEdge, expectedLowEdge, 1.0e-12) || !nearlyEqual(actualUpperEdge, expectedUpperEdge, 1.0e-12)) {
          throw std::runtime_error("v2_2_pt histogram edges do not match v2_2_pt_edges metadata.");
        }

        const double binContent = v2PtHistogramInput->GetBinContent(iBin);
        const double binError = v2PtHistogramInput->GetBinError(iBin);
        if (!isFinite(binContent) || !isFinite(binError) || binError < 0.0) {
          throw std::runtime_error("v2_2_pt bins must have finite contents and finite non-negative errors.");
        }
      }

      std::vector<int> v2EventIds;
      v2EventIds.reserve(v2TracksByEvent.size());
      for (const auto &entry : v2TracksByEvent) {
        v2EventIds.push_back(entry.first);
      }
      std::sort(v2EventIds.begin(), v2EventIds.end());
      blastwave::V2PtCumulant cumulant(v2PtEdges);
      for (int eventId : v2EventIds) {
        cumulant.addEvent(v2TracksByEvent[eventId]);
      }
      const blastwave::V2PtCumulantResult recomputed = cumulant.finalize();
      if (recomputed.v2Values.size() != static_cast<std::size_t>(expectedBins) || recomputed.v2Errors.size() != static_cast<std::size_t>(expectedBins)) {
        throw std::runtime_error("Recomputed v2pt payload shape mismatch.");
      }
      for (int iBin = 1; iBin <= expectedBins; ++iBin) {
        const double expectedValue = recomputed.v2Values[static_cast<std::size_t>(iBin - 1)];
        const double expectedError = recomputed.v2Errors[static_cast<std::size_t>(iBin - 1)];
        const double fileValue = v2PtHistogramInput->GetBinContent(iBin);
        const double fileError = v2PtHistogramInput->GetBinError(iBin);
        if (!nearlyEqual(fileValue, expectedValue, 1.0e-6)) {
          throw std::runtime_error("v2_2_pt content mismatch against recomputed shared-core result.");
        }
        if (!nearlyEqual(fileError, expectedError, 1.0e-6)) {
          throw std::runtime_error("v2_2_pt error mismatch against recomputed shared-core result.");
        }
      }
    }

    // Validate optional gradient debug histograms when present.
    if (hasAnyGradientDebugHistogram) {
      auto validateGradientHistogram = [](const TH2 *histogram, const std::string &name) {
        bool sawPositiveBin = false;
        for (int iBinX = 1; iBinX <= histogram->GetNbinsX(); ++iBinX) {
          for (int iBinY = 1; iBinY <= histogram->GetNbinsY(); ++iBinY) {
            const double binContent = histogram->GetBinContent(iBinX, iBinY);
            if (!std::isfinite(binContent)) {
              throw std::runtime_error(name + " contains NaN/Inf bin content.");
            }
            if (binContent < 0.0) {
              throw std::runtime_error(name + " contains a negative bin.");
            }
            sawPositiveBin = sawPositiveBin || binContent > 0.0;
          }
        }
        if (!sawPositiveBin) {
          throw std::runtime_error(name + " must contain at least one positive bin.");
        }
      };
      validateGradientHistogram(gradientS0Input, "gradient_s0_x-y");
      validateGradientHistogram(gradientSEmInput, "gradient_s_em_x-y");
      validateGradientHistogram(gradientSDynInput, "gradient_s_dyn_x-y");
      validateGradientHistogram(gradientSFInput, "gradient_s_f_x-y");
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
