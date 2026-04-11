#pragma once

#include <RtypesCore.h>

class TTree;

namespace blastwave::io {

// This layer owns the on-disk ROOT tree contract shared by producer and reader
// apps without coupling that serialization detail back into generator records.

inline constexpr char kEventsTreeName[] = "events";
inline constexpr char kParticipantsTreeName[] = "participants";
inline constexpr char kParticlesTreeName[] = "particles";

inline constexpr char kNpartHistogramName[] = "Npart";
inline constexpr char kEps2HistogramName[] = "eps2";
inline constexpr char kPsi2HistogramName[] = "psi2";
inline constexpr char kParticipantXYHistogramName[] = "participant_x-y";
inline constexpr char kParticipantXYCanvasName[] = "participant_x-y_canvas";
inline constexpr char kXYHistogramName[] = "x-y";
inline constexpr char kPxPyHistogramName[] = "px-py";
inline constexpr char kPtHistogramName[] = "pT";
inline constexpr char kEtaHistogramName[] = "eta";
inline constexpr char kPhiHistogramName[] = "phi";

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

struct ParticipantBranches {
  Int_t eventId = 0;
  Int_t nucleusId = 0;
  Double_t x = 0.0;
  Double_t y = 0.0;
  Double_t z = 0.0;
};

void declareEventBranches(TTree& tree, EventBranches& branches);
void declareParticleBranches(TTree& tree, ParticleBranches& branches);
void declareParticipantBranches(TTree& tree, ParticipantBranches& branches);

void bindEventBranches(TTree& tree, EventBranches& branches);
void bindParticleBranches(TTree& tree, ParticleBranches& branches);
void bindParticipantBranches(TTree& tree, ParticipantBranches& branches);

}  // namespace blastwave::io
