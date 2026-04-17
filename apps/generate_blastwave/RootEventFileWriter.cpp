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
#include <stdexcept>
#include <utility>

#include "blastwave/PhysicsUtils.h"
#include "blastwave/io/OutputPathUtils.h"
#include "blastwave/io/RootOutputSchema.h"

namespace {

  double computeParticipantDisplayExtent(const blastwave::BlastWaveConfig &config) {
    const double diffusePadding = 2.0 * config.woodsSaxonDiffuseness + 1.0;
    const double halfSpan = 0.5 * std::abs(config.impactParameter) + config.woodsSaxonRadius + diffusePadding;
    return std::max(15.0, std::ceil(halfSpan));
  }

  blastwave::io::EventBranches toEventBranches(const blastwave::EventInfo &info) {
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
    branches.energy = std::sqrt(branches.mass * branches.mass + branches.px * branches.px + branches.py * branches.py + branches.pz * branches.pz);
    return branches;
  }

  // Prepare the target path before ROOT constructs the output file object.
  const char *prepareRootOutputPath(const std::string &outputPath) {
    blastwave::io::ensureOutputDirectoryExists(outputPath, std::cout);
    return outputPath.c_str();
  }

}  // namespace

namespace blastwave::app {

  struct RootEventFileWriter::Impl {
    Impl(const blastwave::BlastWaveConfig &configValue, const std::string &outputPathValue)
        : config(configValue),
          outputPath(outputPathValue),
          outputFile(prepareRootOutputPath(outputPathValue), "RECREATE"),
          eventsTree(blastwave::io::kEventsTreeName, "Blast-wave event summary"),
          participantsTree(blastwave::io::kParticipantsTreeName, "Participant nucleon records"),
          particlesTree(blastwave::io::kParticlesTreeName, "Blast-wave particle records"),
          hNpart(blastwave::io::kNpartHistogramName, "Participant multiplicity;Npart;Events", 400, -0.5, 399.5),
          hEps2(blastwave::io::kEps2HistogramName, "Participant eccentricity;#epsilon_{2};Events", 100, 0.0, 1.0),
          hPsi2(blastwave::io::kPsi2HistogramName, "Participant-plane angle;#Psi_{2} [rad];Events", 128, -1.7, 1.7),
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

      blastwave::io::declareEventBranches(eventsTree, eventBranches);
      blastwave::io::declareParticipantBranches(participantsTree, participantBranches);
      blastwave::io::declareParticleBranches(particlesTree, particleBranches);
    }

    // Fill the persistent ROOT trees and QA histograms from the generator-owned
    // event record without leaking ROOT types back into the core layer.
    void writeEvent(const blastwave::GeneratedEvent &event) {
      eventBranches = toEventBranches(event.info);
      eventsTree.Fill();

      hNpart.Fill(eventBranches.nParticipants);
      hEps2.Fill(eventBranches.eps2);
      hPsi2.Fill(eventBranches.psi2);
      hCentrality.Fill(eventBranches.centrality);

      for (const blastwave::ParticipantRecord &participant : event.participants) {
        participantBranches = toParticipantBranches(participant);
        participantsTree.Fill();
        hParticipantXY.Fill(participant.x, participant.y);
      }

      for (const blastwave::ParticleRecord &particle : event.particles) {
        particleBranches = toParticleBranches(particle);
        particlesTree.Fill();

        hXY.Fill(particle.x, particle.y);
        hPxPy.Fill(particle.px, particle.py);
        hPt.Fill(std::hypot(particle.px, particle.py));
        hEta.Fill(blastwave::computePseudorapidity(particle.px, particle.py, particle.pz));
        hPhi.Fill(std::atan2(particle.py, particle.px));
      }
    }

    // Finalize the embedded QA objects and persist the full output contract into
    // the target ROOT file exactly once.
    void finish() {
      if (finished) {
        return;
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
      finished = true;
    }

    blastwave::BlastWaveConfig config;
    std::string outputPath;
    bool finished = false;

    TFile outputFile;
    TTree eventsTree;
    TTree participantsTree;
    TTree particlesTree;

    blastwave::io::EventBranches eventBranches;
    blastwave::io::ParticipantBranches participantBranches;
    blastwave::io::ParticleBranches particleBranches;

    TH1F hNpart;
    TH1F hEps2;
    TH1F hPsi2;
    TH1F hCentrality;
    TH2F hParticipantXY;
    TH2F hXY;
    TH2F hPxPy;
    TH1F hPt;
    TH1F hEta;
    TH1F hPhi;
  };

  RootEventFileWriter::RootEventFileWriter(const blastwave::BlastWaveConfig &config, const std::string &outputPath)
      : impl_(std::make_unique<Impl>(config, outputPath)) {}

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
