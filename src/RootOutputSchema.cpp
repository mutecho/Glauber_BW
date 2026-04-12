#include "blastwave/io/RootOutputSchema.h"

#include <TTree.h>

namespace blastwave::io {

namespace {

constexpr char kEventIdBranchName[] = "event_id";
constexpr char kImpactParameterBranchName[] = "b";
constexpr char kNpartBranchName[] = "Npart";
constexpr char kEps2BranchName[] = "eps2";
constexpr char kPsi2BranchName[] = "psi2";
constexpr char kCentralityBranchName[] = "centrality";
constexpr char kNchBranchName[] = "Nch";
constexpr char kNucleusIdBranchName[] = "nucleus_id";
constexpr char kXBranchName[] = "x";
constexpr char kYBranchName[] = "y";
constexpr char kZBranchName[] = "z";
constexpr char kPidBranchName[] = "pid";
constexpr char kChargeBranchName[] = "charge";
constexpr char kMassBranchName[] = "mass";
constexpr char kTBranchName[] = "t";
constexpr char kPxBranchName[] = "px";
constexpr char kPyBranchName[] = "py";
constexpr char kPzBranchName[] = "pz";
constexpr char kEnergyBranchName[] = "E";
constexpr char kEtaSBranchName[] = "eta_s";
constexpr char kSourceXBranchName[] = "source_x";
constexpr char kSourceYBranchName[] = "source_y";

}  // namespace

void declareEventBranches(TTree& tree, EventBranches& branches) {
  tree.Branch(kEventIdBranchName, &branches.eventId, "event_id/I");
  tree.Branch(kImpactParameterBranchName, &branches.impactParameter, "b/D");
  tree.Branch(kNpartBranchName, &branches.nParticipants, "Npart/I");
  tree.Branch(kEps2BranchName, &branches.eps2, "eps2/D");
  tree.Branch(kPsi2BranchName, &branches.psi2, "psi2/D");
  tree.Branch(kCentralityBranchName, &branches.centrality, "centrality/D");
  tree.Branch(kNchBranchName, &branches.nCharged, "Nch/I");
}

void declareParticleBranches(TTree& tree, ParticleBranches& branches) {
  tree.Branch(kEventIdBranchName, &branches.eventId, "event_id/I");
  tree.Branch(kPidBranchName, &branches.pid, "pid/I");
  tree.Branch(kChargeBranchName, &branches.charge, "charge/I");
  tree.Branch(kMassBranchName, &branches.mass, "mass/D");
  tree.Branch(kXBranchName, &branches.x, "x/D");
  tree.Branch(kYBranchName, &branches.y, "y/D");
  tree.Branch(kZBranchName, &branches.z, "z/D");
  tree.Branch(kTBranchName, &branches.t, "t/D");
  tree.Branch(kPxBranchName, &branches.px, "px/D");
  tree.Branch(kPyBranchName, &branches.py, "py/D");
  tree.Branch(kPzBranchName, &branches.pz, "pz/D");
  tree.Branch(kEnergyBranchName, &branches.energy, "E/D");
  tree.Branch(kEtaSBranchName, &branches.etaS, "eta_s/D");
  tree.Branch(kSourceXBranchName, &branches.sourceX, "source_x/D");
  tree.Branch(kSourceYBranchName, &branches.sourceY, "source_y/D");
}

void declareParticipantBranches(TTree& tree, ParticipantBranches& branches) {
  tree.Branch(kEventIdBranchName, &branches.eventId, "event_id/I");
  tree.Branch(kNucleusIdBranchName, &branches.nucleusId, "nucleus_id/I");
  tree.Branch(kXBranchName, &branches.x, "x/D");
  tree.Branch(kYBranchName, &branches.y, "y/D");
  tree.Branch(kZBranchName, &branches.z, "z/D");
}

void bindEventBranches(TTree& tree, EventBranches& branches) {
  tree.SetBranchAddress(kEventIdBranchName, &branches.eventId);
  tree.SetBranchAddress(kImpactParameterBranchName, &branches.impactParameter);
  tree.SetBranchAddress(kNpartBranchName, &branches.nParticipants);
  tree.SetBranchAddress(kEps2BranchName, &branches.eps2);
  tree.SetBranchAddress(kPsi2BranchName, &branches.psi2);
  tree.SetBranchAddress(kCentralityBranchName, &branches.centrality);
  tree.SetBranchAddress(kNchBranchName, &branches.nCharged);
}

void bindParticleBranches(TTree& tree, ParticleBranches& branches) {
  tree.SetBranchAddress(kEventIdBranchName, &branches.eventId);
  tree.SetBranchAddress(kPidBranchName, &branches.pid);
  tree.SetBranchAddress(kChargeBranchName, &branches.charge);
  tree.SetBranchAddress(kMassBranchName, &branches.mass);
  tree.SetBranchAddress(kXBranchName, &branches.x);
  tree.SetBranchAddress(kYBranchName, &branches.y);
  tree.SetBranchAddress(kZBranchName, &branches.z);
  tree.SetBranchAddress(kTBranchName, &branches.t);
  tree.SetBranchAddress(kPxBranchName, &branches.px);
  tree.SetBranchAddress(kPyBranchName, &branches.py);
  tree.SetBranchAddress(kPzBranchName, &branches.pz);
  tree.SetBranchAddress(kEnergyBranchName, &branches.energy);
  tree.SetBranchAddress(kEtaSBranchName, &branches.etaS);
  tree.SetBranchAddress(kSourceXBranchName, &branches.sourceX);
  tree.SetBranchAddress(kSourceYBranchName, &branches.sourceY);
}

void bindParticipantBranches(TTree& tree, ParticipantBranches& branches) {
  tree.SetBranchAddress(kEventIdBranchName, &branches.eventId);
  tree.SetBranchAddress(kNucleusIdBranchName, &branches.nucleusId);
  tree.SetBranchAddress(kXBranchName, &branches.x);
  tree.SetBranchAddress(kYBranchName, &branches.y);
  tree.SetBranchAddress(kZBranchName, &branches.z);
}

}  // namespace blastwave::io
