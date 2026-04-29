#include "generate_blastwave/RootEventFileWriter.h"

#include <TCanvas.h>
#include <TEllipse.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "blastwave/V2PtCumulant.h"
#include "blastwave/FlowFieldModel.h"
#include "blastwave/PhysicsUtils.h"
#include "blastwave/io/OutputPathUtils.h"
#include "blastwave/io/RootOutputSchema.h"
#include "blastwave/io/V2PtRootPayload.h"

namespace {

  double computeParticipantDisplayExtent(const blastwave::BlastWaveConfig &config) {
    const double diffusePadding = 2.0 * config.woodsSaxonDiffuseness + 1.0;
    const double halfSpan = 0.5 * std::abs(config.impactParameter) + config.woodsSaxonRadius + diffusePadding;
    return std::max(15.0, std::ceil(halfSpan));
  }

  bool shouldWriteDensityNormalEventDensity(const blastwave::BlastWaveConfig &config) {
    return config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::DensityNormal;
  }

  std::unique_ptr<TH2F> makeDensityNormalEventDensityHistogram(const blastwave::BlastWaveConfig &config) {
    const double extent = computeParticipantDisplayExtent(config);
    auto histogram = std::make_unique<TH2F>(blastwave::io::kDensityNormalEventDensityHistogramName,
                                            "Density-normal single-event participant density;x [fm];y [fm];#rho(x, y) [fm^{-2}]",
                                            200,
                                            -extent,
                                            extent,
                                            200,
                                            -extent,
                                            extent);
    histogram->SetStats(false);
    histogram->SetOption("LEGO1");
    return histogram;
  }

  std::unique_ptr<TH2F> makeGradientDensityHistogram(const char *name, const char *title, const blastwave::BlastWaveConfig &config) {
    const double extent = computeParticipantDisplayExtent(config);
    auto histogram = std::make_unique<TH2F>(name, title, 200, -extent, extent, 200, -extent, extent);
    histogram->SetStats(false);
    histogram->SetOption("COLZ");
    histogram->SetDirectory(nullptr);
    return histogram;
  }

  // Serialize the generator-owned emission-stage density so optional debug
  // output reflects the same medium state used by flow sampling.
  void fillDensityNormalEventDensityHistogram(TH2F &histogram, const blastwave::GeneratedEvent &event) {
    const std::string histogramTitle = "Density-normal single-event participant density (event " + std::to_string(event.info.eventId)
                                       + ");x [fm];y [fm];#rho(x, y) [fm^{-2}]";
    histogram.SetTitle(histogramTitle.c_str());

    for (int iBinX = 1; iBinX <= histogram.GetNbinsX(); ++iBinX) {
      const double x = histogram.GetXaxis()->GetBinCenter(iBinX);
      for (int iBinY = 1; iBinY <= histogram.GetNbinsY(); ++iBinY) {
        const double y = histogram.GetYaxis()->GetBinCenter(iBinY);
        const blastwave::DensityFieldSample densitySample = blastwave::evaluateDensityField(event.medium.emissionDensity, x, y);
        histogram.SetBinContent(iBinX, iBinY, densitySample.density);
      }
    }
  }

  // Rasterize one analytic density field into a TH2 for optional debug output.
  void fillDensityHistogramFromField(TH2F &histogram, const blastwave::DensityField &field) {
    for (int iBinX = 1; iBinX <= histogram.GetNbinsX(); ++iBinX) {
      const double x = histogram.GetXaxis()->GetBinCenter(iBinX);
      for (int iBinY = 1; iBinY <= histogram.GetNbinsY(); ++iBinY) {
        const double y = histogram.GetYaxis()->GetBinCenter(iBinY);
        const blastwave::DensityFieldSample densitySample = blastwave::evaluateDensityField(field, x, y);
        histogram.SetBinContent(iBinX, iBinY, std::max(0.0, densitySample.density));
      }
    }
  }

  // Fill a discrete freeze-out cloud proxy directly from accepted particle x-y.
  void fillDensityHistogramFromParticleCloud(TH2F &histogram, const blastwave::GeneratedEvent &event) {
    for (const blastwave::ParticleRecord &particle : event.particles) {
      histogram.Fill(particle.x, particle.y, 1.0);
    }
  }

  blastwave::io::EventBranches toEventBranches(const blastwave::EventInfo &info) {
    blastwave::io::EventBranches branches;
    branches.eventId = static_cast<Int_t>(info.eventId);
    branches.impactParameter = info.impactParameter;
    branches.nParticipants = static_cast<Int_t>(info.nParticipants);
    branches.eps2 = info.eps2;
    branches.eps2Freezeout = info.eps2Freezeout;
    branches.psi2 = info.psi2;
    branches.psi2Freezeout = info.psi2Freezeout;
    branches.chi2 = info.chi2;
    branches.r2Initial = info.r2Initial;
    branches.r2Final = info.r2Final;
    branches.r2Ratio = info.r2Ratio;
    branches.v2 = info.v2;
    branches.centrality = info.centrality;
    branches.nCharged = static_cast<Int_t>(info.nCharged);
    return branches;
  }

  blastwave::io::ParticipantBranches toParticipantBranches(const blastwave::ParticipantRecord &participant) {
    blastwave::io::ParticipantBranches branches;
    branches.eventId = static_cast<Int_t>(participant.eventId);
    branches.nucleusId = static_cast<Int_t>(participant.nucleusId);
    branches.x = participant.x;
    branches.y = participant.y;
    branches.z = participant.z;
    return branches;
  }

  blastwave::io::ParticleBranches toParticleBranches(const blastwave::ParticleRecord &particle) {
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
    branches.x0 = particle.x0;
    branches.y0 = particle.y0;
    branches.emissionWeight = particle.emissionWeight;
    branches.energy = std::sqrt(branches.mass * branches.mass + branches.px * branches.px + branches.py * branches.py + branches.pz * branches.pz);
    return branches;
  }

  blastwave::io::FlowEllipseDebugBranches toFlowEllipseDebugBranches(const blastwave::GeneratedEvent &event, const blastwave::BlastWaveConfig &config) {
    blastwave::io::FlowEllipseDebugBranches branches;
    branches.eventId = static_cast<Int_t>(event.info.eventId);
    branches.valid = static_cast<Bool_t>(event.medium.emissionGeometry.valid);
    branches.centerX = event.medium.emissionGeometry.centerX;
    branches.centerY = event.medium.emissionGeometry.centerY;
    branches.sigmaX2 = event.medium.emissionGeometry.sigmaX2;
    branches.sigmaY2 = event.medium.emissionGeometry.sigmaY2;
    branches.sigmaXY = event.medium.emissionGeometry.sigmaXY;
    branches.lambdaMajor = event.medium.emissionGeometry.lambdaMajor;
    branches.lambdaMinor = event.medium.emissionGeometry.lambdaMinor;
    branches.radiusMajor = event.medium.emissionGeometry.radiusMajor;
    branches.radiusMinor = event.medium.emissionGeometry.radiusMinor;
    branches.majorAxisX = event.medium.emissionGeometry.majorAxisX;
    branches.majorAxisY = event.medium.emissionGeometry.majorAxisY;
    branches.minorAxisX = event.medium.emissionGeometry.minorAxisX;
    branches.minorAxisY = event.medium.emissionGeometry.minorAxisY;
    branches.eps2 = event.medium.emissionGeometry.eps2;
    branches.psi2 = event.medium.emissionGeometry.psi2;

    if (config.flowVelocitySamplerMode == blastwave::FlowVelocitySamplerMode::AffineEffective) {
      const blastwave::FlowFieldParameters flowParameters{
          config.flowVelocitySamplerMode,
          config.rho0,
          config.kappa2,
          config.flowPower,
          config.densityNormalKappaCompensation,
          config.affineDeltaTauRef,
          config.affineKappaFlow,
          config.affineKappaAniso,
          config.affineUMax,
      };
      const blastwave::AffineEffectiveFlowInfo affineInfo = blastwave::computeAffineEffectiveFlowInfo(event.medium, flowParameters);
      if (affineInfo.valid) {
        branches.affineEffectiveValid = true;
        branches.affineSigmaIn0 = event.medium.affineEffectiveClosure.sigmaInInitial;
        branches.affineSigmaOut0 = event.medium.affineEffectiveClosure.sigmaOutInitial;
        branches.affineSigmaInF = event.medium.affineEffectiveClosure.sigmaInFinal;
        branches.affineSigmaOutF = event.medium.affineEffectiveClosure.sigmaOutFinal;
        branches.affineGrowthIn = event.medium.affineEffectiveClosure.growthIn;
        branches.affineGrowthOut = event.medium.affineEffectiveClosure.growthOut;
        branches.affineLambdaIn = event.medium.affineEffectiveClosure.lambdaIn;
        branches.affineLambdaOut = event.medium.affineEffectiveClosure.lambdaOut;
        branches.affineLambdaBar = event.medium.affineEffectiveClosure.lambdaBar;
        branches.affineDeltaLambda = event.medium.affineEffectiveClosure.deltaLambda;
        branches.affineHInEff = affineInfo.hInEff;
        branches.affineHOutEff = affineInfo.hOutEff;
        branches.affineUMax = affineInfo.affineUMax;
        branches.affineSurfaceBetaInRaw = affineInfo.surfaceBetaInRaw;
        branches.affineSurfaceBetaOutRaw = affineInfo.surfaceBetaOutRaw;
        branches.affineSurfaceBetaInClipped = affineInfo.surfaceBetaInClipped;
        branches.affineSurfaceBetaOutClipped = affineInfo.surfaceBetaOutClipped;
      }
    }

    return branches;
  }

  // Prepare the target path before ROOT constructs the output file object.
  const char *prepareRootOutputPath(const std::string &outputPath) {
    blastwave::io::ensureOutputDirectoryExists(outputPath, std::cout);
    return outputPath.c_str();
  }

  std::vector<blastwave::V2PtTrack> toV2PtTracks(const std::vector<blastwave::ParticleRecord> &particles) {
    std::vector<blastwave::V2PtTrack> tracks;
    tracks.reserve(particles.size());
    for (const blastwave::ParticleRecord &particle : particles) {
      tracks.push_back({particle.px, particle.py});
    }
    return tracks;
  }

}  // namespace

namespace blastwave::app {

  struct RootEventFileWriter::Impl {
    explicit Impl(const blastwave::app::RunOptions &runOptionsValue)
        : runOptions(runOptionsValue),
          config(runOptionsValue.config),
          outputPath(runOptionsValue.outputPath),
          outputFile(prepareRootOutputPath(runOptionsValue.outputPath), "RECREATE"),
          eventsTree(blastwave::io::kEventsTreeName, "Blast-wave event summary"),
          participantsTree(blastwave::io::kParticipantsTreeName, "Participant nucleon records"),
          particlesTree(blastwave::io::kParticlesTreeName, "Blast-wave particle records"),
          hNpart(blastwave::io::kNpartHistogramName, "Participant multiplicity;Npart;Events", 400, -0.5, 399.5),
          hEps2(blastwave::io::kEps2HistogramName, "Participant eccentricity;#epsilon_{2};Events", 100, 0.0, 1.0),
          hEps2Freezeout(blastwave::io::kEps2FreezeoutHistogramName, "Freeze-out eccentricity;#epsilon_{2}^{f};Events", 100, 0.0, 1.0),
          hPsi2(blastwave::io::kPsi2HistogramName, "Participant-plane angle;#Psi_{2} [rad];Events", 128, -1.7, 1.7),
          hPsi2Freezeout(blastwave::io::kPsi2FreezeoutHistogramName, "Freeze-out participant-plane angle;#Psi_{2}^{f} [rad];Events", 128, -1.7, 1.7),
          hChi2(blastwave::io::kChi2HistogramName, "Freeze-out eccentricity response;#chi_{2} = #epsilon_{2}^{f} / #epsilon_{2};Events", 150, 0.0, 3.0),
          hR2Initial(blastwave::io::kR2InitialHistogramName, "Centered initial marker radius moment;#LT (r_{0}-#LT r_{0}#GT)^{2} #GT [fm^{2}];Events", 160, 0.0, 80.0),
          hR2Final(blastwave::io::kR2FinalHistogramName, "Centered final emission radius moment;#LT (r_{f}-#LT r_{f}#GT)^{2} #GT [fm^{2}];Events", 160, 0.0, 80.0),
          hR2Ratio(blastwave::io::kR2RatioHistogramName, "Radius moment response;#LT r_{f}^{2} #GT / #LT r_{0}^{2} #GT;Events", 200, 0.0, 4.0),
          hV2(blastwave::io::kV2HistogramName, "Event-by-event final-state v_{2};v_{2};Events", 120, 0.0, 1.0),
          hCentrality(blastwave::io::kCentralityHistogramName, "Centrality;centrality [%];Events", 11, 0.0, 110.0),
          hParticipantXY(blastwave::io::kParticipantXYHistogramName,
                         "Participant nucleons;x [fm];y [fm]",
                         200,
                         -computeParticipantDisplayExtent(config),
                         computeParticipantDisplayExtent(config),
                         200,
                         -computeParticipantDisplayExtent(config),
                         computeParticipantDisplayExtent(config)),
          hXY(blastwave::io::kXYHistogramName, "Emission coordinates;x [fm];y [fm]", 120, -15.0, 15.0, 120, -15.0, 15.0),
          hPxPy(blastwave::io::kPxPyHistogramName, "Transverse momentum map;p_{x} [GeV];p_{y} [GeV]", 120, -3.0, 3.0, 120, -3.0, 3.0),
          hPt(blastwave::io::kPtHistogramName, "Transverse momentum;p_{T} [GeV];Particles", 120, 0.0, 3.0),
          hEta(blastwave::io::kEtaHistogramName, "Particle pseudorapidity;#eta;Particles", 120, -6.0, 6.0),
          hPhi(blastwave::io::kPhiHistogramName, "Particle azimuth;#phi;Particles", 128, -3.2, 3.2) {
      gROOT->SetBatch(true);
      gStyle->SetOptStat(1110);

      if (outputFile.IsZombie()) {
        throw std::runtime_error("Failed to create ROOT output file: " + outputPath);
      }

      if (!runOptions.v2PtBinEdges.empty()) {
        v2PtCumulant = std::make_unique<blastwave::V2PtCumulant>(runOptions.v2PtBinEdges);
      }

      blastwave::io::declareEventBranches(eventsTree, eventBranches);
      blastwave::io::declareParticipantBranches(participantsTree, participantBranches);
      blastwave::io::declareParticleBranches(particlesTree, particleBranches);

      // Keep optional flow-ellipse diagnostics in a separate payload so
      // production output stays unchanged unless explicitly requested.
      if (config.debugFlowEllipse) {
        flowEllipseDebugTree = std::make_unique<TTree>(blastwave::io::kFlowEllipseDebugTreeName, "Flow ellipse debug records");
        blastwave::io::declareFlowEllipseDebugBranches(*flowEllipseDebugTree, flowEllipseDebugBranches);
        hFlowEllipseParticipantNormXY = std::make_unique<TH2F>(
            blastwave::io::kFlowEllipseParticipantNormXYHistogramName, "Flow ellipse normalized participant map;x' / R_{major};y' / R_{minor}", 240, -6.0, 6.0, 240, -6.0, 6.0);
      }

      // Keep V2 gradient diagnostics opt-in and first-event-only.
      if (config.debugGradientResponse) {
        hGradientS0 = makeGradientDensityHistogram(blastwave::io::kGradientS0HistogramName,
                                                   "Gradient response s_{0} density;x [fm];y [fm];s_{0}(x,y)",
                                                   config);
        hGradientSEm = makeGradientDensityHistogram(blastwave::io::kGradientSEmHistogramName,
                                                    "Gradient response s_{em} density;x [fm];y [fm];s_{em}(x,y)",
                                                    config);
        hGradientSDyn = makeGradientDensityHistogram(blastwave::io::kGradientSDynHistogramName,
                                                     "Gradient response s_{dyn} density;x [fm];y [fm];s_{dyn}(x,y)",
                                                     config);
        hGradientSF = makeGradientDensityHistogram(blastwave::io::kGradientSFHistogramName,
                                                   "Gradient response sampled s_{f} cloud;x [fm];y [fm];counts",
                                                   config);
      }
    }

    // Fill the persistent ROOT trees and QA histograms from the generator-owned
    // event record without leaking ROOT types back into the core layer.
    void writeEvent(const blastwave::GeneratedEvent &event) {
      eventBranches = toEventBranches(event.info);
      eventsTree.Fill();

      hNpart.Fill(eventBranches.nParticipants);
      hEps2.Fill(eventBranches.eps2);
      hEps2Freezeout.Fill(eventBranches.eps2Freezeout);
      hPsi2.Fill(eventBranches.psi2);
      hPsi2Freezeout.Fill(eventBranches.psi2Freezeout);
      hChi2.Fill(eventBranches.chi2);
      hR2Initial.Fill(eventBranches.r2Initial);
      hR2Final.Fill(eventBranches.r2Final);
      hR2Ratio.Fill(eventBranches.r2Ratio);
      hV2.Fill(eventBranches.v2);
      hCentrality.Fill(eventBranches.centrality);

      if (flowEllipseDebugTree != nullptr) {
        flowEllipseDebugBranches = toFlowEllipseDebugBranches(event, config);
        flowEllipseDebugTree->Fill();
      }

      maybeCaptureDensityNormalEventDensity(event);
      maybeCaptureGradientResponseDebug(event);

      for (const blastwave::ParticipantRecord &participant : event.participants) {
        participantBranches = toParticipantBranches(participant);
        participantsTree.Fill();
        hParticipantXY.Fill(participant.x, participant.y);

        // Build normalized participant coordinates in the principal-axis frame:
        // center-shift -> axis projection -> divide by corresponding semi-axis.
        if (hFlowEllipseParticipantNormXY != nullptr && event.medium.emissionGeometry.valid && event.medium.emissionGeometry.radiusMajor > 0.0
            && event.medium.emissionGeometry.radiusMinor > 0.0 && std::isfinite(event.medium.emissionGeometry.radiusMajor)
            && std::isfinite(event.medium.emissionGeometry.radiusMinor)) {
          const double dx = participant.x - event.medium.emissionGeometry.centerX;
          const double dy = participant.y - event.medium.emissionGeometry.centerY;
          const double xPrime = dx * event.medium.emissionGeometry.majorAxisX + dy * event.medium.emissionGeometry.majorAxisY;
          const double yPrime = dx * event.medium.emissionGeometry.minorAxisX + dy * event.medium.emissionGeometry.minorAxisY;
          const double normalizedX = xPrime / event.medium.emissionGeometry.radiusMajor;
          const double normalizedY = yPrime / event.medium.emissionGeometry.radiusMinor;
          hFlowEllipseParticipantNormXY->Fill(normalizedX, normalizedY);
        }
      }

      for (const blastwave::ParticleRecord &particle : event.particles) {
        particleBranches = toParticleBranches(particle);
        particlesTree.Fill();

        hXY.Fill(particle.x, particle.y);
        hPxPy.Fill(particle.px, particle.py);
        hPt.Fill(std::hypot(particle.px, particle.py));
        hEta.Fill(blastwave::computePseudorapidity(particle.px, particle.py, particle.pz));
        hPhi.Fill(blastwave::computeAzimuth(particle.px, particle.py));
      }

      if (v2PtCumulant != nullptr) {
        v2PtCumulant->addEvent(toV2PtTracks(event.particles));
      }
    }

    // Keep the density-normal visualization sampler-specific and stable by
    // capturing the first event that actually has participant geometry.
    void maybeCaptureDensityNormalEventDensity(const blastwave::GeneratedEvent &event) {
      if (hDensityNormalEventDensity != nullptr || !shouldWriteDensityNormalEventDensity(config) || event.participants.empty()) {
        return;
      }

      hDensityNormalEventDensity = makeDensityNormalEventDensityHistogram(config);
      fillDensityNormalEventDensityHistogram(*hDensityNormalEventDensity, event);
    }

    // Capture optional V2 gradient-response debug payload from the first event
    // with participants so files stay bounded and deterministic.
    void maybeCaptureGradientResponseDebug(const blastwave::GeneratedEvent &event) {
      if (!config.debugGradientResponse || event.participants.empty() || event.particles.empty() || gradientDebugCaptured) {
        return;
      }
      if (hGradientS0 == nullptr || hGradientSEm == nullptr || hGradientSDyn == nullptr || hGradientSF == nullptr) {
        return;
      }

      fillDensityHistogramFromField(*hGradientS0, event.medium.initialDensity);
      fillDensityHistogramFromField(*hGradientSEm, event.medium.markerDensity);
      fillDensityHistogramFromField(*hGradientSDyn, event.medium.dynamicsDensity);
      fillDensityHistogramFromParticleCloud(*hGradientSF, event);
      gradientDebugCaptured = true;
    }

    // Finalize the embedded QA objects and persist the full output contract into
    // the target ROOT file exactly once.
    void finish() {
      if (finished) {
        return;
      }

      std::optional<blastwave::V2PtCumulantResult> v2PtResult;
      if (v2PtCumulant != nullptr) {
        // Finalize the optional analysis before persisting ROOT objects so a
        // failed cumulant does not leave an ambiguous partially written file.
        v2PtResult = v2PtCumulant->finalize();
      }

      TCanvas participantCanvas(blastwave::io::kParticipantXYCanvasName, "Participant nucleons with nucleus outlines", 720, 680);
      participantCanvas.SetRightMargin(0.14F);
      hParticipantXY.SetStats(true);
      hParticipantXY.Draw("COLZ");
      participantCanvas.Modified();
      participantCanvas.Update();

      TEllipse nucleusAOutline(-0.5 * config.impactParameter, 0.0, config.woodsSaxonRadius, config.woodsSaxonRadius);
      nucleusAOutline.SetFillStyle(0);
      nucleusAOutline.SetLineColor(4);
      nucleusAOutline.SetLineWidth(2);
      nucleusAOutline.Draw("same");

      TEllipse nucleusBOutline(+0.5 * config.impactParameter, 0.0, config.woodsSaxonRadius, config.woodsSaxonRadius);
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
      if (flowEllipseDebugTree != nullptr) {
        flowEllipseDebugTree->Write();
        flowEllipseDebugTree->SetDirectory(nullptr);
      }
      hNpart.Write();
      hEps2.Write();
      hEps2Freezeout.Write();
      hPsi2.Write();
      hPsi2Freezeout.Write();
      hChi2.Write();
      hR2Initial.Write();
      hR2Final.Write();
      hR2Ratio.Write();
      hV2.Write();
      hCentrality.Write();
      hParticipantXY.Write();
      if (hFlowEllipseParticipantNormXY != nullptr) {
        hFlowEllipseParticipantNormXY->Write();
        hFlowEllipseParticipantNormXY->SetDirectory(nullptr);
      }
      if (hDensityNormalEventDensity != nullptr) {
        hDensityNormalEventDensity->Write();
        hDensityNormalEventDensity->SetDirectory(nullptr);
      }
      if (gradientDebugCaptured && hGradientS0 != nullptr) {
        hGradientS0->Write();
        hGradientS0->SetDirectory(nullptr);
      }
      if (gradientDebugCaptured && hGradientSEm != nullptr) {
        hGradientSEm->Write();
        hGradientSEm->SetDirectory(nullptr);
      }
      if (gradientDebugCaptured && hGradientSDyn != nullptr) {
        hGradientSDyn->Write();
        hGradientSDyn->SetDirectory(nullptr);
      }
      if (gradientDebugCaptured && hGradientSF != nullptr) {
        hGradientSF->Write();
        hGradientSF->SetDirectory(nullptr);
      }
      hXY.Write();
      hPxPy.Write();
      hPt.Write();
      hEta.Write();
      hPhi.Write();
      participantCanvas.Write();

      if (v2PtResult.has_value()) {
        // Keep pT-bin metadata in the main result file regardless of where the
        // analysis payload is written.
        blastwave::io::writeV2PtEdges(outputFile, v2PtResult->ptBinEdges);
        if (runOptions.v2PtOutputMode == blastwave::app::V2PtOutputMode::SameFile) {
          blastwave::io::writeV2PtPayload(outputFile, *v2PtResult);
        } else {
          blastwave::io::ensureOutputDirectoryExists(runOptions.v2PtOutputPath, std::cout);
          TFile v2PtOutputFile(runOptions.v2PtOutputPath.c_str(), "RECREATE");
          if (v2PtOutputFile.IsZombie()) {
            throw std::runtime_error("Failed to create v2pt output ROOT file: " + runOptions.v2PtOutputPath);
          }
          blastwave::io::writeV2PtPayload(v2PtOutputFile, *v2PtResult);
          v2PtOutputFile.Close();
        }
      }

      outputFile.Close();
      flowEllipseDebugTree.reset();
      hFlowEllipseParticipantNormXY.reset();
      hDensityNormalEventDensity.reset();
      hGradientS0.reset();
      hGradientSEm.reset();
      hGradientSDyn.reset();
      hGradientSF.reset();
      finished = true;
    }

    blastwave::app::RunOptions runOptions;
    blastwave::BlastWaveConfig config;
    std::string outputPath;
    bool finished = false;
    bool gradientDebugCaptured = false;

    TFile outputFile;
    TTree eventsTree;
    TTree participantsTree;
    TTree particlesTree;
    std::unique_ptr<TTree> flowEllipseDebugTree;

    blastwave::io::EventBranches eventBranches;
    blastwave::io::ParticipantBranches participantBranches;
    blastwave::io::ParticleBranches particleBranches;
    blastwave::io::FlowEllipseDebugBranches flowEllipseDebugBranches;

    TH1F hNpart;
    TH1F hEps2;
    TH1F hEps2Freezeout;
    TH1F hPsi2;
    TH1F hPsi2Freezeout;
    TH1F hChi2;
    TH1F hR2Initial;
    TH1F hR2Final;
    TH1F hR2Ratio;
    TH1F hV2;
    TH1F hCentrality;
    TH2F hParticipantXY;
    std::unique_ptr<TH2F> hFlowEllipseParticipantNormXY;
    std::unique_ptr<TH2F> hDensityNormalEventDensity;
    std::unique_ptr<TH2F> hGradientS0;
    std::unique_ptr<TH2F> hGradientSEm;
    std::unique_ptr<TH2F> hGradientSDyn;
    std::unique_ptr<TH2F> hGradientSF;
    TH2F hXY;
    TH2F hPxPy;
    TH1F hPt;
    TH1F hEta;
    TH1F hPhi;
    std::unique_ptr<blastwave::V2PtCumulant> v2PtCumulant;
  };

  RootEventFileWriter::RootEventFileWriter(const blastwave::app::RunOptions &runOptions) : impl_(std::make_unique<Impl>(runOptions)) {
  }

  RootEventFileWriter::~RootEventFileWriter() = default;
  RootEventFileWriter::RootEventFileWriter(RootEventFileWriter &&) noexcept = default;
  RootEventFileWriter &RootEventFileWriter::operator=(RootEventFileWriter &&) noexcept = default;

  void RootEventFileWriter::writeEvent(const blastwave::GeneratedEvent &event) {
    impl_->writeEvent(event);
  }

  void RootEventFileWriter::finish() {
    impl_->finish();
  }

}  // namespace blastwave::app
