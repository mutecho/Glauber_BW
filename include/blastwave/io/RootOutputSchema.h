#pragma once

#include <RtypesCore.h>

class TTree;

namespace blastwave::io {

  // This layer owns the on-disk ROOT tree contract shared by producer and reader
  // apps without coupling that serialization detail back into generator records.

  inline constexpr char kEventsTreeName[] = "events";
  inline constexpr char kParticipantsTreeName[] = "participants";
  inline constexpr char kParticlesTreeName[] = "particles";
  inline constexpr char kFlowEllipseDebugTreeName[] = "flow_ellipse_debug";

  inline constexpr char kNpartHistogramName[] = "Npart";
  inline constexpr char kEps2HistogramName[] = "eps2";
  inline constexpr char kEps2FreezeoutHistogramName[] = "eps2_f";
  inline constexpr char kPsi2HistogramName[] = "psi2";
  inline constexpr char kPsi2FreezeoutHistogramName[] = "psi2_f";
  inline constexpr char kChi2HistogramName[] = "chi2";
  inline constexpr char kR2InitialHistogramName[] = "r2_0";
  inline constexpr char kR2FinalHistogramName[] = "r2_f";
  inline constexpr char kR2RatioHistogramName[] = "r2_ratio";
  inline constexpr char kV2HistogramName[] = "v2";
  inline constexpr char kCentralityHistogramName[] = "cent";
  inline constexpr char kParticipantXYHistogramName[] = "participant_x-y";
  inline constexpr char kFlowEllipseParticipantNormXYHistogramName[] = "flow_ellipse_participant_norm_x-y";
  inline constexpr char kDensityNormalEventDensityHistogramName[] = "density_normal_event_density_x-y";
  inline constexpr char kGradientS0HistogramName[] = "gradient_s0_x-y";
  inline constexpr char kGradientSEmHistogramName[] = "gradient_s_em_x-y";
  inline constexpr char kGradientSDynHistogramName[] = "gradient_s_dyn_x-y";
  inline constexpr char kGradientSFHistogramName[] = "gradient_s_f_x-y";
  inline constexpr char kParticipantXYCanvasName[] = "participant_x-y_canvas";
  inline constexpr char kXYHistogramName[] = "x-y";
  inline constexpr char kPxPyHistogramName[] = "px-py";
  inline constexpr char kPtHistogramName[] = "pT";
  inline constexpr char kEtaHistogramName[] = "eta";
  inline constexpr char kPhiHistogramName[] = "phi";
  inline constexpr char kV2PtEdgesObjectName[] = "v2_2_pt_edges";
  inline constexpr char kV2PtHistogramName[] = "v2_2_pt";
  inline constexpr char kV2PtCanvasName[] = "v2_2_pt_canvas";

  struct EventBranches {
    Int_t eventId = 0;
    Double_t impactParameter = 0.0;
    Int_t nParticipants = 0;
    Double_t eps2 = 0.0;
    Double_t eps2Freezeout = 0.0;
    Double_t psi2 = 0.0;
    Double_t psi2Freezeout = 0.0;
    Double_t chi2 = 0.0;
    Double_t r2Initial = 0.0;
    Double_t r2Final = 0.0;
    Double_t r2Ratio = 0.0;
    Double_t v2 = 0.0;
    Double_t centrality = 0.0;
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
    Double_t x0 = 0.0;
    Double_t y0 = 0.0;
    Double_t emissionWeight = 1.0;
  };

  struct ParticipantBranches {
    Int_t eventId = 0;
    Int_t nucleusId = 0;
    Double_t x = 0.0;
    Double_t y = 0.0;
    Double_t z = 0.0;
  };

  struct FlowEllipseDebugBranches {
    Int_t eventId = 0;
    Bool_t valid = false;
    Double_t centerX = 0.0;
    Double_t centerY = 0.0;
    Double_t sigmaX2 = 0.0;
    Double_t sigmaY2 = 0.0;
    Double_t sigmaXY = 0.0;
    Double_t lambdaMajor = 0.0;
    Double_t lambdaMinor = 0.0;
    Double_t radiusMajor = 0.0;
    Double_t radiusMinor = 0.0;
    Double_t majorAxisX = 1.0;
    Double_t majorAxisY = 0.0;
    Double_t minorAxisX = 0.0;
    Double_t minorAxisY = 1.0;
    Double_t eps2 = 0.0;
    Double_t psi2 = 0.0;
  };

  void declareEventBranches(TTree &tree, EventBranches &branches);
  void declareParticleBranches(TTree &tree, ParticleBranches &branches);
  void declareParticipantBranches(TTree &tree, ParticipantBranches &branches);
  void declareFlowEllipseDebugBranches(TTree &tree, FlowEllipseDebugBranches &branches);

  void bindEventBranches(TTree &tree, EventBranches &branches);
  void bindParticleBranches(TTree &tree, ParticleBranches &branches);
  void bindParticipantBranches(TTree &tree, ParticipantBranches &branches);
  void bindFlowEllipseDebugBranches(TTree &tree, FlowEllipseDebugBranches &branches);

}  // namespace blastwave::io
