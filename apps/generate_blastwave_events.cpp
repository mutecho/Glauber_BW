#include "blastwave/BlastWaveGenerator.h"

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>

#include <cmath>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct EventBranches {
  Int_t eventId = 0;
  Double_t impactParameter = 0.0;
  Int_t nParticipants = 0;
  Double_t eps2 = 0.0;
  Double_t psi2 = 0.0;
  Int_t nCharged = 0;
};

struct ParticleBranches {
  Int_t eventId = 0;
  Int_t pid = 0;
  Int_t charge = 0;
  Double_t mass = 0.0;
  Double_t x = 0.0;
  Double_t y = 0.0;
  Double_t z = 0.0;
  Double_t t = 0.0;
  Double_t px = 0.0;
  Double_t py = 0.0;
  Double_t pz = 0.0;
  Double_t energy = 0.0;
  Double_t etaS = 0.0;
  Double_t sourceX = 0.0;
  Double_t sourceY = 0.0;
};

void printUsage(const char* programName) {
  std::cout
      << "Usage: " << programName << " [options]\n"
      << "Required/primary options:\n"
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

double computePseudorapidity(double px, double py, double pz) {
  const double momentumMagnitude = std::sqrt(px * px + py * py + pz * pz);
  const double denominator = momentumMagnitude - pz;
  if (denominator <= 1.0e-9) {
    return (pz >= 0.0) ? 10.0 : -10.0;
  }
  return 0.5 * std::log((momentumMagnitude + pz) / denominator);
}

std::string takeValue(int& index, int argc, char** argv, const std::string& optionName) {
  if (index + 1 >= argc) {
    throw std::invalid_argument("Missing value for " + optionName);
  }
  ++index;
  return argv[index];
}

}  // namespace

int main(int argc, char** argv) {
  try {
    blastwave::BlastWaveConfig config;
    std::string outputPath = "blastwave.root";

    for (int iArg = 1; iArg < argc; ++iArg) {
      const std::string option = argv[iArg];
      if (option == "--help") {
        printUsage(argv[0]);
        return 0;
      }
      if (option == "--nevents") {
        config.nEvents = std::stoi(takeValue(iArg, argc, argv, option));
      } else if (option == "--b") {
        config.impactParameter = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--temperature") {
        config.temperature = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--tau0") {
        config.tau0 = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--smear") {
        config.smearSigma = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--sigma-nn") {
        config.sigmaNN = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--seed") {
        config.seed =
            static_cast<std::uint64_t>(std::stoull(takeValue(iArg, argc, argv, option)));
      } else if (option == "--output") {
        outputPath = takeValue(iArg, argc, argv, option);
      } else if (option == "--vmax") {
        config.vMax = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--kappa2") {
        config.kappa2 = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--sigma-eta") {
        config.sigmaEta = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--eta-plateau") {
        config.etaPlateauHalfWidth = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--nbd-mu") {
        config.nbdMu = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--nbd-k") {
        config.nbdK = std::stod(takeValue(iArg, argc, argv, option));
      } else if (option == "--r-ref") {
        config.referenceRadius = std::stod(takeValue(iArg, argc, argv, option));
      } else {
        throw std::invalid_argument("Unknown option: " + option);
      }
    }

    blastwave::BlastWaveGenerator generator(config);

    TFile outputFile(outputPath.c_str(), "RECREATE");
    if (outputFile.IsZombie()) {
      throw std::runtime_error("Failed to create ROOT output file: " + outputPath);
    }

    TTree eventsTree("events", "Blast-wave event summary");
    TTree particlesTree("particles", "Blast-wave particle records");

    EventBranches eventBranches;
    ParticleBranches particleBranches;

    eventsTree.Branch("event_id", &eventBranches.eventId, "event_id/I");
    eventsTree.Branch("b", &eventBranches.impactParameter, "b/D");
    eventsTree.Branch("Npart", &eventBranches.nParticipants, "Npart/I");
    eventsTree.Branch("eps2", &eventBranches.eps2, "eps2/D");
    eventsTree.Branch("psi2", &eventBranches.psi2, "psi2/D");
    eventsTree.Branch("Nch", &eventBranches.nCharged, "Nch/I");

    particlesTree.Branch("event_id", &particleBranches.eventId, "event_id/I");
    particlesTree.Branch("pid", &particleBranches.pid, "pid/I");
    particlesTree.Branch("charge", &particleBranches.charge, "charge/I");
    particlesTree.Branch("mass", &particleBranches.mass, "mass/D");
    particlesTree.Branch("x", &particleBranches.x, "x/D");
    particlesTree.Branch("y", &particleBranches.y, "y/D");
    particlesTree.Branch("z", &particleBranches.z, "z/D");
    particlesTree.Branch("t", &particleBranches.t, "t/D");
    particlesTree.Branch("px", &particleBranches.px, "px/D");
    particlesTree.Branch("py", &particleBranches.py, "py/D");
    particlesTree.Branch("pz", &particleBranches.pz, "pz/D");
    particlesTree.Branch("E", &particleBranches.energy, "E/D");
    particlesTree.Branch("eta_s", &particleBranches.etaS, "eta_s/D");
    particlesTree.Branch("source_x", &particleBranches.sourceX, "source_x/D");
    particlesTree.Branch("source_y", &particleBranches.sourceY, "source_y/D");

    TH1F hNpart("Npart", "Participant multiplicity;Npart;Events", 400, -0.5, 399.5);
    TH1F hEps2("eps2", "Participant eccentricity;#epsilon_{2};Events", 100, 0.0, 1.0);
    TH1F hPsi2("psi2", "Participant-plane angle;#Psi_{2} [rad];Events", 128, -1.7, 1.7);
    TH2F hXY("x-y", "Emission coordinates;x [fm];y [fm]", 120, -15.0, 15.0, 120, -15.0, 15.0);
    TH2F hPxPy("px-py", "Transverse momentum map;p_{x} [GeV];p_{y} [GeV]", 120, -3.0, 3.0, 120,
               -3.0, 3.0);
    TH1F hPt("pT", "Transverse momentum;p_{T} [GeV];Particles", 120, 0.0, 3.0);
    TH1F hEta("eta", "Particle pseudorapidity;#eta;Particles", 120, -6.0, 6.0);
    TH1F hPhi("phi", "Particle azimuth;#phi;Particles", 128, -3.2, 3.2);

    for (int eventId = 0; eventId < config.nEvents; ++eventId) {
      const blastwave::GeneratedEvent event = generator.generateEvent(eventId);

      eventBranches.eventId = static_cast<Int_t>(event.info.eventId);
      eventBranches.impactParameter = event.info.impactParameter;
      eventBranches.nParticipants = static_cast<Int_t>(event.info.nParticipants);
      eventBranches.eps2 = event.info.eps2;
      eventBranches.psi2 = event.info.psi2;
      eventBranches.nCharged = static_cast<Int_t>(event.info.nCharged);
      eventsTree.Fill();

      hNpart.Fill(eventBranches.nParticipants);
      hEps2.Fill(eventBranches.eps2);
      hPsi2.Fill(eventBranches.psi2);

      for (const blastwave::ParticleRecord& particle : event.particles) {
        particleBranches.eventId = static_cast<Int_t>(particle.eventId);
        particleBranches.pid = static_cast<Int_t>(particle.pid);
        particleBranches.charge = static_cast<Int_t>(particle.charge);
        particleBranches.mass = particle.mass;
        particleBranches.x = particle.x;
        particleBranches.y = particle.y;
        particleBranches.z = particle.z;
        particleBranches.t = particle.t;
        particleBranches.px = particle.px;
        particleBranches.py = particle.py;
        particleBranches.pz = particle.pz;
        particleBranches.etaS = particle.etaS;
        particleBranches.sourceX = particle.sourceX;
        particleBranches.sourceY = particle.sourceY;
        particleBranches.energy = std::sqrt(
            particleBranches.mass * particleBranches.mass +
            particleBranches.px * particleBranches.px +
            particleBranches.py * particleBranches.py +
            particleBranches.pz * particleBranches.pz);
        particlesTree.Fill();

        const double pt = std::hypot(particle.px, particle.py);
        hXY.Fill(particle.x, particle.y);
        hPxPy.Fill(particle.px, particle.py);
        hPt.Fill(pt);
        hEta.Fill(computePseudorapidity(particle.px, particle.py, particle.pz));
        hPhi.Fill(std::atan2(particle.py, particle.px));
      }
    }

    outputFile.cd();
    eventsTree.Write();
    particlesTree.Write();
    hNpart.Write();
    hEps2.Write();
    hPsi2.Write();
    hXY.Write();
    hPxPy.Write();
    hPt.Write();
    hEta.Write();
    hPhi.Write();
    outputFile.Close();

    std::cout << "Wrote " << config.nEvents << " events to " << outputPath << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "generate_blastwave_events failed: " << error.what() << '\n';
    return 1;
  }
}
