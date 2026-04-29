#include "blastwave/io/RootOutputSchema.h"

#include <TTree.h>

namespace blastwave::io {

  namespace {

    constexpr char kEventIdBranchName[] = "event_id";
    constexpr char kImpactParameterBranchName[] = "b";
    constexpr char kNpartBranchName[] = "Npart";
    constexpr char kEps2BranchName[] = "eps2";
    constexpr char kEps2FreezeoutBranchName[] = "eps2_f";
    constexpr char kPsi2BranchName[] = "psi2";
    constexpr char kPsi2FreezeoutBranchName[] = "psi2_f";
    constexpr char kChi2BranchName[] = "chi2";
    constexpr char kR2InitialBranchName[] = "r2_0";
    constexpr char kR2FinalBranchName[] = "r2_f";
    constexpr char kR2RatioBranchName[] = "r2_ratio";
    constexpr char kV2BranchName[] = "v2";
    constexpr char kCentralityBranchName[] = "centrality";
    constexpr char kNchBranchName[] = "Nch";
    constexpr char kNucleusIdBranchName[] = "nucleus_id";
    constexpr char kXBranchName[] = "x";
    constexpr char kYBranchName[] = "y";
    constexpr char kZBranchName[] = "z";
    constexpr char kPidBranchName[] = "pid";
    constexpr char kChargeBranchName[] = "charge";
    constexpr char kMassBranchName[] = "mass";
    constexpr char kValidBranchName[] = "valid";
    constexpr char kTBranchName[] = "t";
    constexpr char kPxBranchName[] = "px";
    constexpr char kPyBranchName[] = "py";
    constexpr char kPzBranchName[] = "pz";
    constexpr char kEnergyBranchName[] = "E";
    constexpr char kEtaSBranchName[] = "eta_s";
    constexpr char kSourceXBranchName[] = "source_x";
    constexpr char kSourceYBranchName[] = "source_y";
    constexpr char kX0BranchName[] = "x0";
    constexpr char kY0BranchName[] = "y0";
    constexpr char kEmissionWeightBranchName[] = "emission_weight";
    constexpr char kCenterXBranchName[] = "center_x";
    constexpr char kCenterYBranchName[] = "center_y";
    constexpr char kSigmaX2BranchName[] = "sigma_x2";
    constexpr char kSigmaY2BranchName[] = "sigma_y2";
    constexpr char kSigmaXYBranchName[] = "sigma_xy";
    constexpr char kLambdaMajorBranchName[] = "lambda_major";
    constexpr char kLambdaMinorBranchName[] = "lambda_minor";
    constexpr char kRadiusMajorBranchName[] = "R_major";
    constexpr char kRadiusMinorBranchName[] = "R_minor";
    constexpr char kMajorAxisXBranchName[] = "major_axis_x";
    constexpr char kMajorAxisYBranchName[] = "major_axis_y";
    constexpr char kMinorAxisXBranchName[] = "minor_axis_x";
    constexpr char kMinorAxisYBranchName[] = "minor_axis_y";
    constexpr char kAffineEffectiveValidBranchName[] = "affine_effective_valid";
    constexpr char kAffineSigmaIn0BranchName[] = "affine_sigma_in_0";
    constexpr char kAffineSigmaOut0BranchName[] = "affine_sigma_out_0";
    constexpr char kAffineSigmaInFBranchName[] = "affine_sigma_in_f";
    constexpr char kAffineSigmaOutFBranchName[] = "affine_sigma_out_f";
    constexpr char kAffineGrowthInBranchName[] = "affine_growth_in";
    constexpr char kAffineGrowthOutBranchName[] = "affine_growth_out";
    constexpr char kAffineLambdaInBranchName[] = "affine_lambda_in";
    constexpr char kAffineLambdaOutBranchName[] = "affine_lambda_out";
    constexpr char kAffineLambdaBarBranchName[] = "affine_lambda_bar";
    constexpr char kAffineDeltaLambdaBranchName[] = "affine_delta_lambda";
    constexpr char kAffineHInEffBranchName[] = "affine_h_in_eff";
    constexpr char kAffineHOutEffBranchName[] = "affine_h_out_eff";
    constexpr char kAffineUMaxBranchName[] = "affine_u_max";
    constexpr char kAffineSurfaceBetaInRawBranchName[] = "affine_surface_beta_in_raw";
    constexpr char kAffineSurfaceBetaOutRawBranchName[] = "affine_surface_beta_out_raw";
    constexpr char kAffineSurfaceBetaInClippedBranchName[] = "affine_surface_beta_in_clipped";
    constexpr char kAffineSurfaceBetaOutClippedBranchName[] = "affine_surface_beta_out_clipped";

  }  // namespace

  void declareEventBranches(TTree &tree, EventBranches &branches) {
    tree.Branch(kEventIdBranchName, &branches.eventId, "event_id/I");
    tree.Branch(kImpactParameterBranchName, &branches.impactParameter, "b/D");
    tree.Branch(kNpartBranchName, &branches.nParticipants, "Npart/I");
    tree.Branch(kEps2BranchName, &branches.eps2, "eps2/D");
    tree.Branch(kEps2FreezeoutBranchName, &branches.eps2Freezeout, "eps2_f/D");
    tree.Branch(kPsi2BranchName, &branches.psi2, "psi2/D");
    tree.Branch(kPsi2FreezeoutBranchName, &branches.psi2Freezeout, "psi2_f/D");
    tree.Branch(kChi2BranchName, &branches.chi2, "chi2/D");
    tree.Branch(kR2InitialBranchName, &branches.r2Initial, "r2_0/D");
    tree.Branch(kR2FinalBranchName, &branches.r2Final, "r2_f/D");
    tree.Branch(kR2RatioBranchName, &branches.r2Ratio, "r2_ratio/D");
    tree.Branch(kV2BranchName, &branches.v2, "v2/D");
    tree.Branch(kCentralityBranchName, &branches.centrality, "centrality/D");
    tree.Branch(kNchBranchName, &branches.nCharged, "Nch/I");
  }

  void declareParticleBranches(TTree &tree, ParticleBranches &branches) {
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
    tree.Branch(kX0BranchName, &branches.x0, "x0/D");
    tree.Branch(kY0BranchName, &branches.y0, "y0/D");
    tree.Branch(kEmissionWeightBranchName, &branches.emissionWeight, "emission_weight/D");
  }

  void declareParticipantBranches(TTree &tree, ParticipantBranches &branches) {
    tree.Branch(kEventIdBranchName, &branches.eventId, "event_id/I");
    tree.Branch(kNucleusIdBranchName, &branches.nucleusId, "nucleus_id/I");
    tree.Branch(kXBranchName, &branches.x, "x/D");
    tree.Branch(kYBranchName, &branches.y, "y/D");
    tree.Branch(kZBranchName, &branches.z, "z/D");
  }

  void declareFlowEllipseDebugBranches(TTree &tree, FlowEllipseDebugBranches &branches) {
    tree.Branch(kEventIdBranchName, &branches.eventId, "event_id/I");
    tree.Branch(kValidBranchName, &branches.valid, "valid/O");
    tree.Branch(kCenterXBranchName, &branches.centerX, "center_x/D");
    tree.Branch(kCenterYBranchName, &branches.centerY, "center_y/D");
    tree.Branch(kSigmaX2BranchName, &branches.sigmaX2, "sigma_x2/D");
    tree.Branch(kSigmaY2BranchName, &branches.sigmaY2, "sigma_y2/D");
    tree.Branch(kSigmaXYBranchName, &branches.sigmaXY, "sigma_xy/D");
    tree.Branch(kLambdaMajorBranchName, &branches.lambdaMajor, "lambda_major/D");
    tree.Branch(kLambdaMinorBranchName, &branches.lambdaMinor, "lambda_minor/D");
    tree.Branch(kRadiusMajorBranchName, &branches.radiusMajor, "radius_major/D");
    tree.Branch(kRadiusMinorBranchName, &branches.radiusMinor, "radius_minor/D");
    tree.Branch(kMajorAxisXBranchName, &branches.majorAxisX, "major_axis_x/D");
    tree.Branch(kMajorAxisYBranchName, &branches.majorAxisY, "major_axis_y/D");
    tree.Branch(kMinorAxisXBranchName, &branches.minorAxisX, "minor_axis_x/D");
    tree.Branch(kMinorAxisYBranchName, &branches.minorAxisY, "minor_axis_y/D");
    tree.Branch(kEps2BranchName, &branches.eps2, "eps2/D");
    tree.Branch(kPsi2BranchName, &branches.psi2, "psi2/D");
    tree.Branch(kAffineEffectiveValidBranchName, &branches.affineEffectiveValid, "affine_effective_valid/O");
    tree.Branch(kAffineSigmaIn0BranchName, &branches.affineSigmaIn0, "affine_sigma_in_0/D");
    tree.Branch(kAffineSigmaOut0BranchName, &branches.affineSigmaOut0, "affine_sigma_out_0/D");
    tree.Branch(kAffineSigmaInFBranchName, &branches.affineSigmaInF, "affine_sigma_in_f/D");
    tree.Branch(kAffineSigmaOutFBranchName, &branches.affineSigmaOutF, "affine_sigma_out_f/D");
    tree.Branch(kAffineGrowthInBranchName, &branches.affineGrowthIn, "affine_growth_in/D");
    tree.Branch(kAffineGrowthOutBranchName, &branches.affineGrowthOut, "affine_growth_out/D");
    tree.Branch(kAffineLambdaInBranchName, &branches.affineLambdaIn, "affine_lambda_in/D");
    tree.Branch(kAffineLambdaOutBranchName, &branches.affineLambdaOut, "affine_lambda_out/D");
    tree.Branch(kAffineLambdaBarBranchName, &branches.affineLambdaBar, "affine_lambda_bar/D");
    tree.Branch(kAffineDeltaLambdaBranchName, &branches.affineDeltaLambda, "affine_delta_lambda/D");
    tree.Branch(kAffineHInEffBranchName, &branches.affineHInEff, "affine_h_in_eff/D");
    tree.Branch(kAffineHOutEffBranchName, &branches.affineHOutEff, "affine_h_out_eff/D");
    tree.Branch(kAffineUMaxBranchName, &branches.affineUMax, "affine_u_max/D");
    tree.Branch(kAffineSurfaceBetaInRawBranchName, &branches.affineSurfaceBetaInRaw, "affine_surface_beta_in_raw/D");
    tree.Branch(kAffineSurfaceBetaOutRawBranchName, &branches.affineSurfaceBetaOutRaw, "affine_surface_beta_out_raw/D");
    tree.Branch(kAffineSurfaceBetaInClippedBranchName, &branches.affineSurfaceBetaInClipped, "affine_surface_beta_in_clipped/D");
    tree.Branch(kAffineSurfaceBetaOutClippedBranchName, &branches.affineSurfaceBetaOutClipped, "affine_surface_beta_out_clipped/D");
  }

  void bindEventBranches(TTree &tree, EventBranches &branches) {
    tree.SetBranchAddress(kEventIdBranchName, &branches.eventId);
    tree.SetBranchAddress(kImpactParameterBranchName, &branches.impactParameter);
    tree.SetBranchAddress(kNpartBranchName, &branches.nParticipants);
    tree.SetBranchAddress(kEps2BranchName, &branches.eps2);
    tree.SetBranchAddress(kEps2FreezeoutBranchName, &branches.eps2Freezeout);
    tree.SetBranchAddress(kPsi2BranchName, &branches.psi2);
    tree.SetBranchAddress(kPsi2FreezeoutBranchName, &branches.psi2Freezeout);
    tree.SetBranchAddress(kChi2BranchName, &branches.chi2);
    tree.SetBranchAddress(kR2InitialBranchName, &branches.r2Initial);
    tree.SetBranchAddress(kR2FinalBranchName, &branches.r2Final);
    tree.SetBranchAddress(kR2RatioBranchName, &branches.r2Ratio);
    tree.SetBranchAddress(kV2BranchName, &branches.v2);
    tree.SetBranchAddress(kCentralityBranchName, &branches.centrality);
    tree.SetBranchAddress(kNchBranchName, &branches.nCharged);
  }

  void bindParticleBranches(TTree &tree, ParticleBranches &branches) {
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
    tree.SetBranchAddress(kX0BranchName, &branches.x0);
    tree.SetBranchAddress(kY0BranchName, &branches.y0);
    tree.SetBranchAddress(kEmissionWeightBranchName, &branches.emissionWeight);
  }

  void bindParticipantBranches(TTree &tree, ParticipantBranches &branches) {
    tree.SetBranchAddress(kEventIdBranchName, &branches.eventId);
    tree.SetBranchAddress(kNucleusIdBranchName, &branches.nucleusId);
    tree.SetBranchAddress(kXBranchName, &branches.x);
    tree.SetBranchAddress(kYBranchName, &branches.y);
    tree.SetBranchAddress(kZBranchName, &branches.z);
  }

  void bindFlowEllipseDebugBranches(TTree &tree, FlowEllipseDebugBranches &branches) {
    tree.SetBranchAddress(kEventIdBranchName, &branches.eventId);
    tree.SetBranchAddress(kValidBranchName, &branches.valid);
    tree.SetBranchAddress(kCenterXBranchName, &branches.centerX);
    tree.SetBranchAddress(kCenterYBranchName, &branches.centerY);
    tree.SetBranchAddress(kSigmaX2BranchName, &branches.sigmaX2);
    tree.SetBranchAddress(kSigmaY2BranchName, &branches.sigmaY2);
    tree.SetBranchAddress(kSigmaXYBranchName, &branches.sigmaXY);
    tree.SetBranchAddress(kLambdaMajorBranchName, &branches.lambdaMajor);
    tree.SetBranchAddress(kLambdaMinorBranchName, &branches.lambdaMinor);
    tree.SetBranchAddress(kRadiusMajorBranchName, &branches.radiusMajor);
    tree.SetBranchAddress(kRadiusMinorBranchName, &branches.radiusMinor);
    tree.SetBranchAddress(kMajorAxisXBranchName, &branches.majorAxisX);
    tree.SetBranchAddress(kMajorAxisYBranchName, &branches.majorAxisY);
    tree.SetBranchAddress(kMinorAxisXBranchName, &branches.minorAxisX);
    tree.SetBranchAddress(kMinorAxisYBranchName, &branches.minorAxisY);
    tree.SetBranchAddress(kEps2BranchName, &branches.eps2);
    tree.SetBranchAddress(kPsi2BranchName, &branches.psi2);
    tree.SetBranchAddress(kAffineEffectiveValidBranchName, &branches.affineEffectiveValid);
    tree.SetBranchAddress(kAffineSigmaIn0BranchName, &branches.affineSigmaIn0);
    tree.SetBranchAddress(kAffineSigmaOut0BranchName, &branches.affineSigmaOut0);
    tree.SetBranchAddress(kAffineSigmaInFBranchName, &branches.affineSigmaInF);
    tree.SetBranchAddress(kAffineSigmaOutFBranchName, &branches.affineSigmaOutF);
    tree.SetBranchAddress(kAffineGrowthInBranchName, &branches.affineGrowthIn);
    tree.SetBranchAddress(kAffineGrowthOutBranchName, &branches.affineGrowthOut);
    tree.SetBranchAddress(kAffineLambdaInBranchName, &branches.affineLambdaIn);
    tree.SetBranchAddress(kAffineLambdaOutBranchName, &branches.affineLambdaOut);
    tree.SetBranchAddress(kAffineLambdaBarBranchName, &branches.affineLambdaBar);
    tree.SetBranchAddress(kAffineDeltaLambdaBranchName, &branches.affineDeltaLambda);
    tree.SetBranchAddress(kAffineHInEffBranchName, &branches.affineHInEff);
    tree.SetBranchAddress(kAffineHOutEffBranchName, &branches.affineHOutEff);
    tree.SetBranchAddress(kAffineUMaxBranchName, &branches.affineUMax);
    tree.SetBranchAddress(kAffineSurfaceBetaInRawBranchName, &branches.affineSurfaceBetaInRaw);
    tree.SetBranchAddress(kAffineSurfaceBetaOutRawBranchName, &branches.affineSurfaceBetaOutRaw);
    tree.SetBranchAddress(kAffineSurfaceBetaInClippedBranchName, &branches.affineSurfaceBetaInClipped);
    tree.SetBranchAddress(kAffineSurfaceBetaOutClippedBranchName, &branches.affineSurfaceBetaOutClipped);
  }

}  // namespace blastwave::io
