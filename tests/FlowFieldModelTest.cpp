#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "blastwave/EventMedium.h"
#include "blastwave/EmissionSampler.h"
#include "blastwave/FlowFieldModel.h"

namespace {

  constexpr double kTolerance = 1.0e-9;

  void require(bool condition, const std::string &message) {
    if (!condition) {
      throw std::runtime_error(message);
    }
  }

  void requireNear(double actual, double expected, double tolerance, const std::string &message) {
    if (std::abs(actual - expected) > tolerance) {
      throw std::runtime_error(message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
    }
  }

  std::vector<blastwave::WeightedTransversePoint> makeAxisAlignedEllipse() {
    return {
        {-2.0, -1.0, 1.0},
        {-2.0, 1.0, 1.0},
        {2.0, -1.0, 1.0},
        {2.0, 1.0, 1.0},
    };
  }

  std::vector<blastwave::WeightedTransversePoint> makeRotatedEllipse(double angle) {
    const std::vector<blastwave::WeightedTransversePoint> base = makeAxisAlignedEllipse();
    std::vector<blastwave::WeightedTransversePoint> rotated;
    rotated.reserve(base.size());
    for (const blastwave::WeightedTransversePoint &point : base) {
      rotated.push_back({
          point.x * std::cos(angle) - point.y * std::sin(angle),
          point.x * std::sin(angle) + point.y * std::cos(angle),
          point.weight,
      });
    }
    return rotated;
  }

  blastwave::EventMedium buildAxisAlignedMedium(double densitySigma = 0.5) {
    return blastwave::buildEventMedium(makeAxisAlignedEllipse(), {blastwave::DensityEvolutionMode::None, densitySigma});
  }

  blastwave::EventMedium buildAffineAxisAlignedMedium(double densitySigma = 0.5) {
    return blastwave::buildEventMedium(
        makeAxisAlignedEllipse(), {blastwave::DensityEvolutionMode::AffineGaussianResponse, densitySigma, 1.20, 1.05, 0.5});
  }

  blastwave::EventMedium buildAffineRotatedMedium(double angle, double densitySigma = 0.5) {
    return blastwave::buildEventMedium(
        makeRotatedEllipse(angle), {blastwave::DensityEvolutionMode::AffineGaussianResponse, densitySigma, 1.20, 1.05, 0.5});
  }

  void runAxisAlignedEllipseRecoveryTest() {
    const blastwave::FlowEllipseInfo ellipse = blastwave::computeFlowEllipseInfo(makeAxisAlignedEllipse());
    require(ellipse.valid, "Axis-aligned ellipse should be valid.");
    requireNear(ellipse.centerX, 0.0, kTolerance, "Axis-aligned centerX mismatch.");
    requireNear(ellipse.centerY, 0.0, kTolerance, "Axis-aligned centerY mismatch.");
    requireNear(ellipse.sigmaX2, 4.0, kTolerance, "Axis-aligned sigmaX2 mismatch.");
    requireNear(ellipse.sigmaY2, 1.0, kTolerance, "Axis-aligned sigmaY2 mismatch.");
    requireNear(ellipse.sigmaXY, 0.0, kTolerance, "Axis-aligned sigmaXY mismatch.");
    requireNear(ellipse.lambdaMajor, 4.0, kTolerance, "Axis-aligned lambdaMajor mismatch.");
    requireNear(ellipse.lambdaMinor, 1.0, kTolerance, "Axis-aligned lambdaMinor mismatch.");
    requireNear(ellipse.radiusMajor, 2.0, kTolerance, "Axis-aligned radiusMajor mismatch.");
    requireNear(ellipse.radiusMinor, 1.0, kTolerance, "Axis-aligned radiusMinor mismatch.");
    requireNear(ellipse.inverseSigmaXX, 0.25, kTolerance, "Axis-aligned inverseSigmaXX mismatch.");
    requireNear(ellipse.inverseSigmaXY, 0.0, kTolerance, "Axis-aligned inverseSigmaXY mismatch.");
    requireNear(ellipse.inverseSigmaYY, 1.0, kTolerance, "Axis-aligned inverseSigmaYY mismatch.");
    requireNear(std::hypot(ellipse.majorAxisX, ellipse.majorAxisY), 1.0, kTolerance, "Major axis is not normalized.");
    requireNear(std::hypot(ellipse.minorAxisX, ellipse.minorAxisY), 1.0, kTolerance, "Minor axis is not normalized.");
    requireNear(ellipse.majorAxisX * ellipse.minorAxisX + ellipse.majorAxisY * ellipse.minorAxisY,
                0.0,
                kTolerance,
                "Axis-aligned principal axes must be orthogonal.");
    requireNear(ellipse.minorAxisX, 0.0, kTolerance, "Axis-aligned minor axis x mismatch.");
    requireNear(ellipse.minorAxisY, 1.0, kTolerance, "Axis-aligned minor axis y mismatch.");
  }

  void runRotatedEllipseRecoveryTest() {
    const double angle = 0.37;
    const blastwave::FlowEllipseInfo ellipse = blastwave::computeFlowEllipseInfo(makeRotatedEllipse(angle));
    require(ellipse.valid, "Rotated ellipse should be valid.");
    requireNear(ellipse.lambdaMajor, 4.0, 1.0e-8, "Rotated lambdaMajor mismatch.");
    requireNear(ellipse.lambdaMinor, 1.0, 1.0e-8, "Rotated lambdaMinor mismatch.");
    requireNear(std::hypot(ellipse.majorAxisX, ellipse.majorAxisY), 1.0, kTolerance, "Rotated major axis is not normalized.");
    requireNear(std::hypot(ellipse.minorAxisX, ellipse.minorAxisY), 1.0, kTolerance, "Rotated minor axis is not normalized.");
    requireNear(ellipse.majorAxisX * ellipse.minorAxisX + ellipse.majorAxisY * ellipse.minorAxisY,
                0.0,
                kTolerance,
                "Rotated principal axes must be orthogonal.");

    const double expectedMinorX = std::cos(ellipse.psi2);
    const double expectedMinorY = std::sin(ellipse.psi2);
    require(ellipse.minorAxisX * expectedMinorX + ellipse.minorAxisY * expectedMinorY >= -kTolerance,
            "Minor axis orientation should agree with psi2.");
  }

  void runInsufficientParticipantsTest() {
    const blastwave::FlowEllipseInfo empty = blastwave::computeFlowEllipseInfo({});
    require(!empty.valid, "Empty participant cloud must be invalid.");

    const blastwave::FlowEllipseInfo single = blastwave::computeFlowEllipseInfo({{3.0, -2.0, 1.0}});
    require(!single.valid, "Single participant cloud must be invalid.");
    requireNear(single.centerX, 3.0, kTolerance, "Single-point centerX mismatch.");
    requireNear(single.centerY, -2.0, kTolerance, "Single-point centerY mismatch.");
  }

  void runDegenerateLineTest() {
    const blastwave::FlowEllipseInfo ellipse = blastwave::computeFlowEllipseInfo({
        {-1.0, 0.0, 1.0},
        {0.0, 0.0, 1.0},
        {1.0, 0.0, 1.0},
    });
    require(!ellipse.valid, "Line-like participant cloud must be invalid.");
    requireNear(ellipse.lambdaMajor, 2.0 / 3.0, kTolerance, "Degenerate line lambdaMajor mismatch.");
    requireNear(ellipse.lambdaMinor, 0.0, kTolerance, "Degenerate line lambdaMinor mismatch.");
  }

  void runSingleGaussianDensityTest() {
    const blastwave::DensityField field = blastwave::buildGaussianPointCloudDensityField({{0.0, 0.0, 1.0}}, 0.5);
    const blastwave::DensityFieldSample sample = blastwave::evaluateDensityField(field, 0.4, -0.3);
    const double sigma2 = 0.25;
    const double normalization = 1.0 / (2.0 * std::acos(-1.0) * sigma2);
    const double expectedDensity = normalization * std::exp(-0.5 * ((0.4 * 0.4 + (-0.3) * (-0.3)) / sigma2));
    requireNear(sample.density, expectedDensity, 1.0e-12, "Single-Gaussian density mismatch.");
    requireNear(sample.gradientX, -expectedDensity * 0.4 / sigma2, 1.0e-12, "Single-Gaussian gradientX mismatch.");
    requireNear(sample.gradientY, -expectedDensity * -0.3 / sigma2, 1.0e-12, "Single-Gaussian gradientY mismatch.");
    require(sample.gradientX * 0.4 + sample.gradientY * -0.3 < 0.0, "Single-Gaussian gradient should point back toward the origin.");
  }

  void runInvalidDensitySigmaTest() {
    const blastwave::DensityField field = blastwave::buildGaussianPointCloudDensityField({{0.0, 0.0, 1.0}}, 0.0, 0.0, 0.0);
    const blastwave::DensityFieldSample sample = blastwave::evaluateDensityField(field, 0.0, 0.0);
    requireNear(sample.density, 0.0, 1.0e-12, "Invalid density sigma should return zero density.");
    requireNear(sample.gradientX, 0.0, 1.0e-12, "Invalid density sigma should return zero gradientX.");
    requireNear(sample.gradientY, 0.0, 1.0e-12, "Invalid density sigma should return zero gradientY.");
  }

  void runFullCovarianceDensityTest() {
    const blastwave::DensityField field = blastwave::buildGaussianPointCloudDensityField({{0.0, 0.0, 1.0}}, 0.5, 0.1, 0.3);
    const blastwave::DensityFieldSample sample = blastwave::evaluateDensityField(field, 0.2, -0.4);

    const double determinant = 0.5 * 0.3 - 0.1 * 0.1;
    const double inverseCovXX = 0.3 / determinant;
    const double inverseCovXY = -0.1 / determinant;
    const double inverseCovYY = 0.5 / determinant;
    const double qx = inverseCovXX * 0.2 + inverseCovXY * (-0.4);
    const double qy = inverseCovXY * 0.2 + inverseCovYY * (-0.4);
    const double expectedDensity = (1.0 / (2.0 * std::acos(-1.0) * std::sqrt(determinant))) * std::exp(-0.5 * (0.2 * qx + (-0.4) * qy));
    requireNear(sample.density, expectedDensity, 1.0e-12, "Full-covariance density mismatch.");
    requireNear(sample.gradientX, -expectedDensity * qx, 1.0e-12, "Full-covariance gradientX mismatch.");
    requireNear(sample.gradientY, -expectedDensity * qy, 1.0e-12, "Full-covariance gradientY mismatch.");
  }

  void runDensityWeightFilteringTest() {
    const blastwave::DensityField filtered = blastwave::buildGaussianPointCloudDensityField(
        {{0.0, 0.0, 2.0}, {1.0, 0.0, 0.0}, {2.0, 0.0, -1.0}, {3.0, 0.0, std::numeric_limits<double>::quiet_NaN()}}, 0.5);
    const blastwave::DensityField positiveOnly = blastwave::buildGaussianPointCloudDensityField({{0.0, 0.0, 2.0}}, 0.5);
    const blastwave::DensityFieldSample filteredSample = blastwave::evaluateDensityField(filtered, 0.2, 0.0);
    const blastwave::DensityFieldSample positiveOnlySample = blastwave::evaluateDensityField(positiveOnly, 0.2, 0.0);
    requireNear(filteredSample.density, positiveOnlySample.density, 1.0e-12, "Density should ignore non-positive or non-finite weights.");
    requireNear(filteredSample.gradientX, positiveOnlySample.gradientX, 1.0e-12, "Filtered density gradientX mismatch.");
    requireNear(filteredSample.gradientY, positiveOnlySample.gradientY, 1.0e-12, "Filtered density gradientY mismatch.");
  }

  void runCovarianceEllipseDirectionTest() {
    const blastwave::EventMedium medium = buildAxisAlignedMedium();
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
        0.4,
        0.0,
        1.0,
    };
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(medium, 1.0, 0.5, parameters);
    require(sample.betaT > 0.0, "Covariance-ellipse test needs a non-zero flow sample.");

    const double normalization = std::sqrt((1.0 / 4.0) * (1.0 / 4.0) + (0.5 / 1.0) * (0.5 / 1.0));
    const double expectedNormalX = (1.0 / 4.0) / normalization;
    const double expectedNormalY = (0.5 / 1.0) / normalization;
    const double directionX = sample.betaX / sample.betaT;
    const double directionY = sample.betaY / sample.betaT;
    const double dot = directionX * expectedNormalX + directionY * expectedNormalY;
    requireNear(dot, 1.0, 1.0e-9, "Covariance-ellipse sampler should follow the ellipse normal when kappa2=0.");
  }

  void runDensityNormalDirectionTest() {
    const blastwave::EventMedium medium = buildAxisAlignedMedium(0.35);
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::DensityNormal,
        0.4,
        0.0,
        1.0,
    };
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(medium, 2.2, 0.0, parameters);
    require(sample.betaT > 0.0, "Density-normal test needs a non-zero flow sample.");
    requireNear(sample.betaX / sample.betaT, 1.0, 1.0e-6, "Density-normal sampler should point outward along +x in this symmetric probe.");
    requireNear(sample.betaY / sample.betaT, 0.0, 1.0e-6, "Density-normal sampler should stay on the x axis in this symmetric probe.");
  }

  void runDensityNormalCenterFallbackTest() {
    const blastwave::EventMedium medium = buildAxisAlignedMedium();
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::DensityNormal,
        0.4,
        0.0,
        1.0,
    };
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(medium, 0.0, 0.0, parameters);
    requireNear(sample.betaT, 0.0, 1.0e-12, "Density-normal center fallback should return zero flow when both gradient and ellipse fallback vanish.");
  }

  void runDensityNormalIgnoresKappa2Test() {
    const blastwave::EventMedium medium = buildAxisAlignedMedium(0.35);
    const blastwave::FlowFieldSample sampleA = blastwave::evaluateFlowField(
        medium,
        2.2,
        0.0,
        {blastwave::FlowVelocitySamplerMode::DensityNormal, 0.7, 0.0, 1.0});
    const blastwave::FlowFieldSample sampleB = blastwave::evaluateFlowField(
        medium,
        2.2,
        0.0,
        {blastwave::FlowVelocitySamplerMode::DensityNormal, 0.7, 100.0, 1.0});
    requireNear(sampleA.betaX, sampleB.betaX, 1.0e-12, "Legacy density-normal betaX must ignore kappa2.");
    requireNear(sampleA.betaY, sampleB.betaY, 1.0e-12, "Legacy density-normal betaY must ignore kappa2.");
    requireNear(sampleA.rhoRaw, sampleB.rhoRaw, 1.0e-12, "Legacy density-normal rhoRaw must ignore kappa2.");
  }

  void runSharedRTildeTest() {
    const blastwave::EventMedium medium = buildAxisAlignedMedium(0.35);
    const blastwave::FlowFieldSample covarianceSample = blastwave::evaluateFlowField(
        medium,
        1.0,
        0.5,
        {blastwave::FlowVelocitySamplerMode::CovarianceEllipse, 0.4, 0.0, 1.0});
    const blastwave::FlowFieldSample densitySample = blastwave::evaluateFlowField(
        medium,
        1.0,
        0.5,
        {blastwave::FlowVelocitySamplerMode::DensityNormal, 0.4, 0.0, 1.0});
    requireNear(covarianceSample.rTilde, densitySample.rTilde, 1.0e-12, "Both samplers should share the same covariance-based rTilde.");
  }

  void runGradientResponseSiteOverloadTest() {
    const blastwave::EventMedium medium = buildAxisAlignedMedium();
    blastwave::EmissionSite site;
    site.betaTX = 0.3;
    site.betaTY = 0.4;

    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(
        medium,
        site,
        {blastwave::FlowVelocitySamplerMode::GradientResponse, 0.8, 0.4, 1.2});
    requireNear(sample.betaX, 0.3, kTolerance, "Gradient-response overload should preserve site betaX.");
    requireNear(sample.betaY, 0.4, kTolerance, "Gradient-response overload should preserve site betaY.");
    requireNear(sample.betaT, 0.5, kTolerance, "Gradient-response overload should preserve the transverse beta magnitude.");
    requireNear(sample.phiB, std::atan2(0.4, 0.3), kTolerance, "Gradient-response overload should preserve phiB.");
    requireNear(sample.rhoRaw, std::atanh(0.5), kTolerance, "Gradient-response overload should map betaT through atanh.");

    blastwave::EmissionSite fastSite;
    fastSite.betaTX = 0.97;
    const blastwave::FlowFieldSample fastSample = blastwave::evaluateFlowField(
        medium,
        fastSite,
        {blastwave::FlowVelocitySamplerMode::GradientResponse, 0.8, 0.4, 1.2});
    requireNear(fastSample.betaT, 0.97, kTolerance, "Gradient-response overload should not apply the legacy 0.95 clip.");
  }

  void runAffineMediumGeometryTest() {
    const blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    requireNear(medium.participantGeometry.sigmaX2, 4.0, 1.0e-12, "Participant sigmaX2 must preserve initial geometry.");
    requireNear(medium.participantGeometry.sigmaY2, 1.0, 1.0e-12, "Participant sigmaY2 must preserve initial geometry.");

    const double expectedSupportSigmaX2 = 4.0 * 1.05 * 1.05;
    const double expectedSupportSigmaY2 = 1.0 * 1.20 * 1.20;
    const double expectedKernelSigmaX2 = 0.25 * 1.05 * 1.05 + 0.25;
    const double expectedKernelSigmaY2 = 0.25 * 1.20 * 1.20 + 0.25;
    requireNear(medium.emissionGeometry.sigmaX2, expectedSupportSigmaX2 + expectedKernelSigmaX2, 1.0e-10, "Affine emission sigmaX2 mismatch.");
    requireNear(medium.emissionGeometry.sigmaY2, expectedSupportSigmaY2 + expectedKernelSigmaY2, 1.0e-10, "Affine emission sigmaY2 mismatch.");
    require(medium.emissionGeometry.eps2 < medium.participantGeometry.eps2, "Affine response should reduce eps2 for the fixed V1a parameters in this probe.");
    const double chi2 = medium.participantGeometry.eps2 > 1.0e-12 ? medium.emissionGeometry.eps2 / medium.participantGeometry.eps2 : 0.0;
    require(chi2 > 0.0 && chi2 < 1.0, "Affine response should yield 0<chi2<1 in this axis-aligned probe.");
  }

  void runAffineEffectiveClosureTest() {
    const blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    require(medium.affineEffectiveClosure.valid, "Affine-effective closure should be valid for the affine-gaussian baseline.");
    requireNear(medium.affineEffectiveClosure.sigmaInInitial, 1.0, 1.0e-12, "Affine-effective sigma_in_0 mismatch.");
    requireNear(medium.affineEffectiveClosure.sigmaOutInitial, 2.0, 1.0e-12, "Affine-effective sigma_out_0 mismatch.");
    requireNear(medium.affineEffectiveClosure.sigmaInFinal, medium.emissionGeometry.radiusMinor, 1.0e-12, "Affine-effective sigma_in_f mismatch.");
    requireNear(medium.affineEffectiveClosure.sigmaOutFinal, medium.emissionGeometry.radiusMajor, 1.0e-12, "Affine-effective sigma_out_f mismatch.");
    requireNear(medium.affineEffectiveClosure.growthIn,
                medium.affineEffectiveClosure.sigmaInFinal / medium.affineEffectiveClosure.sigmaInInitial,
                1.0e-12,
                "Affine-effective growth_in mismatch.");
    requireNear(medium.affineEffectiveClosure.growthOut,
                medium.affineEffectiveClosure.sigmaOutFinal / medium.affineEffectiveClosure.sigmaOutInitial,
                1.0e-12,
                "Affine-effective growth_out mismatch.");
    requireNear(medium.affineEffectiveClosure.lambdaBar,
                0.5 * (medium.affineEffectiveClosure.lambdaIn + medium.affineEffectiveClosure.lambdaOut),
                1.0e-12,
                "Affine-effective lambdaBar mismatch.");
    requireNear(medium.affineEffectiveClosure.deltaLambda,
                0.5 * (medium.affineEffectiveClosure.lambdaIn - medium.affineEffectiveClosure.lambdaOut),
                1.0e-12,
                "Affine-effective deltaLambda mismatch.");
  }

  void runAffineEffectiveFlowInfoTest() {
    const blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    const blastwave::FlowFieldParameters additiveParameters{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.8,
        0.0,
        1.0,
        false,
        10.0,
        10.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::AdditiveRho,
    };
    const blastwave::AffineEffectiveFlowInfo additiveInfo = blastwave::computeAffineEffectiveFlowInfo(medium, additiveParameters);
    require(additiveInfo.valid, "Affine-effective additive flow info should be valid for a well-formed affine medium.");
    require(additiveInfo.affineEffectiveMode == blastwave::AffineEffectiveMode::AdditiveRho,
            "Affine-effective info should expose additive-rho mode.");
    requireNear(additiveInfo.hInEff, medium.affineEffectiveClosure.lambdaIn / additiveParameters.affineDeltaTauRef, 1.0e-12, "hInEff semantics mismatch.");
    requireNear(additiveInfo.hOutEff, medium.affineEffectiveClosure.lambdaOut / additiveParameters.affineDeltaTauRef, 1.0e-12, "hOutEff semantics mismatch.");
    requireNear(additiveInfo.surfaceBetaInRaw, std::tanh(additiveInfo.surfaceRhoTotalIn), 1.0e-12, "Additive surface beta in raw mismatch.");
    requireNear(additiveInfo.surfaceBetaOutRaw, std::tanh(additiveInfo.surfaceRhoTotalOut), 1.0e-12, "Additive surface beta out raw mismatch.");
    requireNear(additiveInfo.surfaceBetaInClipped, std::min(additiveInfo.surfaceBetaInRaw, additiveParameters.affineUMax), 1.0e-12, "Additive in clipping mismatch.");
    requireNear(additiveInfo.surfaceBetaOutClipped, std::min(additiveInfo.surfaceBetaOutRaw, additiveParameters.affineUMax), 1.0e-12, "Additive out clipping mismatch.");

    const blastwave::FlowFieldParameters fullTensorParameters{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.8,
        0.0,
        1.0,
        false,
        10.0,
        10.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::FullTensor,
    };
    const blastwave::AffineEffectiveFlowInfo fullTensorInfo = blastwave::computeAffineEffectiveFlowInfo(medium, fullTensorParameters);
    require(fullTensorInfo.valid, "Affine-effective full-tensor flow info should stay valid.");
    require(fullTensorInfo.affineEffectiveMode == blastwave::AffineEffectiveMode::FullTensor,
            "Affine-effective info should expose full-tensor mode.");
    requireNear(fullTensorInfo.surfaceBetaInRaw,
                std::abs(fullTensorParameters.affineKappaFlow * fullTensorInfo.hInEff * medium.affineEffectiveClosure.sigmaInFinal),
                1.0e-12,
                "Full-tensor surface beta in raw mismatch.");
    requireNear(fullTensorInfo.surfaceBetaOutRaw,
                std::abs(fullTensorParameters.affineKappaFlow * fullTensorInfo.hOutEff * medium.affineEffectiveClosure.sigmaOutFinal),
                1.0e-12,
                "Full-tensor surface beta out raw mismatch.");
  }

  void runAffineFlowResponseTest() {
    blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    // Deliberately perturb the freeze-out diagnostics after geometry setup so
    // the formula test fails if V1a ever uses eps2_f/psi2_f instead of the
    // initial participant eccentricity vector.
    medium.emissionGeometry.eps2 = 0.05;
    medium.emissionGeometry.psi2 = medium.participantGeometry.psi2 + 0.37;
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
        0.8,
        0.4,
        1.2,
    };
    const blastwave::FlowFieldSample covarianceSample = blastwave::evaluateFlowField(medium, 2.0, 0.0, parameters);
    require(covarianceSample.betaT > 0.0, "Affine covariance sample should produce non-zero betaT.");

    const double a2 = parameters.kappa2 * medium.participantGeometry.eps2;
    const double expectedCovarianceRhoRaw = parameters.rho0 * std::pow(covarianceSample.rTilde, parameters.flowPower)
                                            * std::exp(2.0 * a2 * std::cos(2.0 * (covarianceSample.phiB - medium.participantGeometry.psi2)));
    requireNear(covarianceSample.rhoRaw, expectedCovarianceRhoRaw, 1.0e-12, "Affine covariance kappa2*eps2 rhoRaw formula mismatch.");

    const blastwave::FlowFieldSample densityNormalSample = blastwave::evaluateFlowField(
        medium, 2.0, 0.0, {blastwave::FlowVelocitySamplerMode::DensityNormal, 0.8, 0.4, 1.2});
    require(densityNormalSample.betaT > 0.0, "Affine density-normal sample should produce non-zero betaT.");
    const double expectedDensityRhoRaw = 0.8 * std::pow(densityNormalSample.rTilde, 1.2);
    requireNear(densityNormalSample.rhoRaw, expectedDensityRhoRaw, 1.0e-12, "Affine density-normal default rhoRaw formula mismatch.");
  }

  void runAffineDensityNormalCompensationTest() {
    blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    medium.emissionGeometry.eps2 = 0.05;
    medium.emissionGeometry.psi2 = medium.participantGeometry.psi2 + 0.37;

    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(
        medium, 2.0, 0.0, {blastwave::FlowVelocitySamplerMode::DensityNormal, 0.8, 0.4, 1.2, true});
    require(sample.betaT > 0.0, "Affine density-normal compensation test should produce non-zero betaT.");

    const double a2 = 0.4 * medium.participantGeometry.eps2;
    const double expectedRhoRaw = 0.8 * std::pow(sample.rTilde, 1.2)
                                  * std::exp(2.0 * a2 * std::cos(2.0 * (sample.phiB - medium.participantGeometry.psi2)));
    requireNear(sample.rhoRaw, expectedRhoRaw, 1.0e-12, "Affine density-normal compensation rhoRaw formula mismatch.");
  }

  void runAffineEffectiveAdditiveDirectionAndCenterTest() {
    const blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.8,
        0.0,
        1.0,
        false,
        10.0,
        10.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::AdditiveRho,
    };
    const blastwave::FlowFieldSample directionSample = blastwave::evaluateFlowField(medium, 2.2, 0.0, parameters);
    require(directionSample.betaT > 0.0, "Additive-rho direction test needs non-zero flow.");
    requireNear(directionSample.betaX / directionSample.betaT, 1.0, 1.0e-6, "Additive-rho should follow density-normal +x direction.");
    requireNear(directionSample.betaY / directionSample.betaT, 0.0, 1.0e-6, "Additive-rho should follow density-normal y component.");

    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(
        medium, medium.emissionGeometry.centerX, medium.emissionGeometry.centerY, parameters);
    requireNear(sample.betaT, 0.0, 1.0e-12, "Affine-effective center probe should return zero transverse flow.");
  }

  void runAffineEffectiveAdditiveFallbackDirectionTest() {
    blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    medium.emissionDensity.gaussianSigma = 0.0;
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.8,
        0.0,
        1.0,
        false,
        10.0,
        10.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::AdditiveRho,
    };
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(medium, 1.0, 0.5, parameters);
    require(sample.betaT > 0.0, "Additive fallback direction test needs non-zero flow.");

    // The fallback normal belongs to the evolved emissionGeometry, not the
    // initial participant ellipse, because affine-effective samples at freeze-out.
    const double deltaX = 1.0 - medium.emissionGeometry.centerX;
    const double deltaY = 0.5 - medium.emissionGeometry.centerY;
    const double qX = medium.emissionGeometry.inverseSigmaXX * deltaX + medium.emissionGeometry.inverseSigmaXY * deltaY;
    const double qY = medium.emissionGeometry.inverseSigmaXY * deltaX + medium.emissionGeometry.inverseSigmaYY * deltaY;
    const double normalization = std::hypot(qX, qY);
    const double expectedNormalX = qX / normalization;
    const double expectedNormalY = qY / normalization;
    requireNear(sample.betaX / sample.betaT, expectedNormalX, 1.0e-6, "Additive fallback should use ellipse normal x.");
    requireNear(sample.betaY / sample.betaT, expectedNormalY, 1.0e-6, "Additive fallback should use ellipse normal y.");
  }

  void runAffineEffectiveAdditiveRhoBaselineAndContrastTest() {
    const blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    const blastwave::FlowFieldParameters noBaseline{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.0,
        0.0,
        1.0,
        false,
        10.0,
        10.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::AdditiveRho,
    };
    const blastwave::FlowFieldParameters withBaseline{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.9,
        0.0,
        1.0,
        false,
        10.0,
        10.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::AdditiveRho,
    };
    const blastwave::FlowFieldSample sampleNoBaseline = blastwave::evaluateFlowField(medium, 2.0, 0.0, noBaseline);
    const blastwave::FlowFieldSample sampleWithBaseline = blastwave::evaluateFlowField(medium, 2.0, 0.0, withBaseline);
    require(sampleWithBaseline.betaT > sampleNoBaseline.betaT, "Additive-rho rho0 baseline should increase mean flow magnitude.");

    const blastwave::FlowFieldParameters noCorrection{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.8,
        0.0,
        1.0,
        false,
        10.0,
        0.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::AdditiveRho,
    };
    const blastwave::FlowFieldParameters withCorrection{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.8,
        0.0,
        1.0,
        false,
        10.0,
        10.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::AdditiveRho,
    };
    const double inX = medium.emissionGeometry.centerX + medium.emissionGeometry.radiusMinor * medium.emissionGeometry.minorAxisX;
    const double inY = medium.emissionGeometry.centerY + medium.emissionGeometry.radiusMinor * medium.emissionGeometry.minorAxisY;
    const double outX = medium.emissionGeometry.centerX + medium.emissionGeometry.radiusMajor * medium.emissionGeometry.majorAxisX;
    const double outY = medium.emissionGeometry.centerY + medium.emissionGeometry.radiusMajor * medium.emissionGeometry.majorAxisY;
    const double anisotropyNoCorrection =
        std::abs(blastwave::evaluateFlowField(medium, inX, inY, noCorrection).betaT - blastwave::evaluateFlowField(medium, outX, outY, noCorrection).betaT);
    const double anisotropyWithCorrection =
        std::abs(blastwave::evaluateFlowField(medium, inX, inY, withCorrection).betaT - blastwave::evaluateFlowField(medium, outX, outY, withCorrection).betaT);
    require(anisotropyWithCorrection > anisotropyNoCorrection + 1.0e-6,
            "Additive-rho affine correction should increase principal-axis flow anisotropy at fixed rho0.");
  }

  void runAffineEffectiveSurfaceClippingTest() {
    const blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    const blastwave::FlowFieldParameters additiveParameters{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.8,
        0.0,
        1.0,
        false,
        1.0,
        10.0,
        1.0,
        0.20,
        blastwave::AffineEffectiveMode::AdditiveRho,
    };
    const blastwave::AffineEffectiveFlowInfo flowInfo = blastwave::computeAffineEffectiveFlowInfo(medium, additiveParameters);
    require(flowInfo.valid, "Affine-effective surface clipping diagnostic should be valid.");
    require(flowInfo.surfaceBetaInRaw > additiveParameters.affineUMax || flowInfo.surfaceBetaOutRaw > additiveParameters.affineUMax,
            "Affine-effective clipping test needs at least one raw surface beta above affine-u-max.");

    const double probeX = medium.emissionGeometry.centerX + medium.emissionGeometry.radiusMinor * medium.emissionGeometry.minorAxisX;
    const double probeY = medium.emissionGeometry.centerY + medium.emissionGeometry.radiusMinor * medium.emissionGeometry.minorAxisY;
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(medium, probeX, probeY, additiveParameters);
    requireNear(sample.betaT, additiveParameters.affineUMax, 1.0e-12, "Additive-rho probe should clip exactly to affine-u-max.");

    const blastwave::FlowFieldParameters fullTensorParameters{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.8,
        0.0,
        1.0,
        false,
        1.0,
        10.0,
        1.0,
        0.20,
        blastwave::AffineEffectiveMode::FullTensor,
    };
    const blastwave::FlowFieldSample fullTensorSample = blastwave::evaluateFlowField(medium, probeX, probeY, fullTensorParameters);
    requireNear(fullTensorSample.betaT, fullTensorParameters.affineUMax, 1.0e-12, "Full-tensor probe should clip exactly to affine-u-max.");
  }

  void runAffineEffectiveFullTensorFormulaAndRotationTest() {
    const blastwave::EventMedium axisMedium = buildAffineAxisAlignedMedium();
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::AffineEffective,
        0.3,
        0.0,
        1.0,
        false,
        10.0,
        10.0,
        0.0,
        0.95,
        blastwave::AffineEffectiveMode::FullTensor,
    };
    const double probeX = axisMedium.emissionGeometry.centerX + 0.6 * axisMedium.emissionGeometry.radiusMinor * axisMedium.emissionGeometry.minorAxisX
                          + 0.4 * axisMedium.emissionGeometry.radiusMajor * axisMedium.emissionGeometry.majorAxisX;
    const double probeY = axisMedium.emissionGeometry.centerY + 0.6 * axisMedium.emissionGeometry.radiusMinor * axisMedium.emissionGeometry.minorAxisY
                          + 0.4 * axisMedium.emissionGeometry.radiusMajor * axisMedium.emissionGeometry.majorAxisY;
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(axisMedium, probeX, probeY, parameters);
    const double hIn = axisMedium.affineEffectiveClosure.lambdaIn / parameters.affineDeltaTauRef;
    const double hOut = axisMedium.affineEffectiveClosure.lambdaOut / parameters.affineDeltaTauRef;
    const double xPrime = (probeX - axisMedium.emissionGeometry.centerX) * axisMedium.emissionGeometry.minorAxisX
                          + (probeY - axisMedium.emissionGeometry.centerY) * axisMedium.emissionGeometry.minorAxisY;
    const double yPrime = (probeX - axisMedium.emissionGeometry.centerX) * axisMedium.emissionGeometry.majorAxisX
                          + (probeY - axisMedium.emissionGeometry.centerY) * axisMedium.emissionGeometry.majorAxisY;
    const double radialProfile = std::pow(sample.rTilde, parameters.flowPower);
    const double uXPrime = parameters.affineKappaFlow * radialProfile * hIn * xPrime;
    const double uYPrime = parameters.affineKappaFlow * radialProfile * hOut * yPrime;
    const double expectedBetaX = uXPrime * axisMedium.emissionGeometry.minorAxisX + uYPrime * axisMedium.emissionGeometry.majorAxisX;
    const double expectedBetaY = uXPrime * axisMedium.emissionGeometry.minorAxisY + uYPrime * axisMedium.emissionGeometry.majorAxisY;
    requireNear(sample.betaX, expectedBetaX, 1.0e-10, "Full-tensor betaX formula mismatch.");
    requireNear(sample.betaY, expectedBetaY, 1.0e-10, "Full-tensor betaY formula mismatch.");
    requireNear(sample.phiB, std::atan2(sample.betaY, sample.betaX), 1.0e-12, "Full-tensor phiB mismatch.");

    const blastwave::EventMedium rotatedMedium = buildAffineRotatedMedium(0.37);
    const double rotatedProbeX = rotatedMedium.emissionGeometry.centerX + 0.7 * rotatedMedium.emissionGeometry.radiusMinor * rotatedMedium.emissionGeometry.minorAxisX
                                 + 0.3 * rotatedMedium.emissionGeometry.radiusMajor * rotatedMedium.emissionGeometry.majorAxisX;
    const double rotatedProbeY = rotatedMedium.emissionGeometry.centerY + 0.7 * rotatedMedium.emissionGeometry.radiusMinor * rotatedMedium.emissionGeometry.minorAxisY
                                 + 0.3 * rotatedMedium.emissionGeometry.radiusMajor * rotatedMedium.emissionGeometry.majorAxisY;
    const blastwave::FlowFieldSample rotatedSample = blastwave::evaluateFlowField(rotatedMedium, rotatedProbeX, rotatedProbeY, parameters);
    require(rotatedSample.betaT > 0.0, "Full-tensor rotation test needs non-zero flow.");

    const blastwave::FlowFieldSample centerSample = blastwave::evaluateFlowField(
        axisMedium, axisMedium.emissionGeometry.centerX, axisMedium.emissionGeometry.centerY, parameters);
    requireNear(centerSample.betaT, 0.0, 1.0e-12, "Full-tensor center probe should return zero transverse flow.");
  }

  void runAffineEffectiveKappaAnisoNoOpTest() {
    const blastwave::EventMedium medium = buildAffineAxisAlignedMedium();
    const blastwave::FlowFieldSample additiveA = blastwave::evaluateFlowField(
        medium, 1.8, 0.2, {blastwave::FlowVelocitySamplerMode::AffineEffective, 0.8, 0.0, 1.0, false, 10.0, 10.0, 0.0, 0.95, blastwave::AffineEffectiveMode::AdditiveRho});
    const blastwave::FlowFieldSample additiveB = blastwave::evaluateFlowField(
        medium, 1.8, 0.2, {blastwave::FlowVelocitySamplerMode::AffineEffective, 0.8, 0.0, 1.0, false, 10.0, 10.0, 999.0, 0.95, blastwave::AffineEffectiveMode::AdditiveRho});
    requireNear(additiveA.betaX, additiveB.betaX, 1.0e-12, "Additive-rho betaX must ignore affine-kappa-aniso.");
    requireNear(additiveA.betaY, additiveB.betaY, 1.0e-12, "Additive-rho betaY must ignore affine-kappa-aniso.");
    requireNear(additiveA.rhoRaw, additiveB.rhoRaw, 1.0e-12, "Additive-rho rhoRaw must ignore affine-kappa-aniso.");

    const blastwave::FlowFieldSample fullTensorA = blastwave::evaluateFlowField(
        medium, 1.8, 0.2, {blastwave::FlowVelocitySamplerMode::AffineEffective, 0.8, 0.0, 1.0, false, 10.0, 10.0, 0.0, 0.95, blastwave::AffineEffectiveMode::FullTensor});
    const blastwave::FlowFieldSample fullTensorB = blastwave::evaluateFlowField(
        medium, 1.8, 0.2, {blastwave::FlowVelocitySamplerMode::AffineEffective, 0.8, 0.0, 1.0, false, 10.0, 10.0, 999.0, 0.95, blastwave::AffineEffectiveMode::FullTensor});
    requireNear(fullTensorA.betaX, fullTensorB.betaX, 1.0e-12, "Full-tensor betaX must ignore affine-kappa-aniso.");
    requireNear(fullTensorA.betaY, fullTensorB.betaY, 1.0e-12, "Full-tensor betaY must ignore affine-kappa-aniso.");
    requireNear(fullTensorA.rhoRaw, fullTensorB.rhoRaw, 1.0e-12, "Full-tensor rhoRaw must ignore affine-kappa-aniso.");
  }

  void runLegacyCovarianceKappa2ResponseTest() {
    const blastwave::EventMedium medium = buildAxisAlignedMedium();
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
        0.6,
        0.3,
        1.1,
    };
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(medium, 1.3, 0.4, parameters);
    require(sample.betaT > 0.0, "Legacy covariance kappa2 response test needs non-zero flow.");

    const double a2 = parameters.kappa2 * medium.participantGeometry.eps2;
    const double expectedRhoRaw = std::pow(sample.rTilde, parameters.flowPower) * (parameters.rho0 + a2 * std::cos(2.0 * sample.phiB));
    requireNear(sample.rhoRaw, expectedRhoRaw, 1.0e-12, "Legacy covariance kappa2*eps2 rhoRaw formula mismatch.");
  }

  void runVelocityClippingTest() {
    const blastwave::EventMedium medium = buildAxisAlignedMedium();
    const blastwave::FlowFieldParameters parameters{
        blastwave::FlowVelocitySamplerMode::CovarianceEllipse,
        20.0,
        20.0,
        1.0,
    };
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(medium, 2.0, 0.0, parameters);
    require(sample.betaT < 1.0, "Clipped betaT must stay subluminal.");
    requireNear(sample.betaT, 0.95, 1.0e-12, "betaT clipping threshold mismatch.");
  }

}  // namespace

int main() {
  try {
    runAxisAlignedEllipseRecoveryTest();
    runRotatedEllipseRecoveryTest();
    runInsufficientParticipantsTest();
    runDegenerateLineTest();
    runSingleGaussianDensityTest();
    runInvalidDensitySigmaTest();
    runFullCovarianceDensityTest();
    runDensityWeightFilteringTest();
    runAffineMediumGeometryTest();
    runAffineEffectiveClosureTest();
    runAffineEffectiveFlowInfoTest();
    runCovarianceEllipseDirectionTest();
    runDensityNormalDirectionTest();
    runDensityNormalCenterFallbackTest();
    runDensityNormalIgnoresKappa2Test();
    runAffineFlowResponseTest();
    runAffineDensityNormalCompensationTest();
    runAffineEffectiveAdditiveDirectionAndCenterTest();
    runAffineEffectiveAdditiveFallbackDirectionTest();
    runAffineEffectiveAdditiveRhoBaselineAndContrastTest();
    runAffineEffectiveSurfaceClippingTest();
    runAffineEffectiveFullTensorFormulaAndRotationTest();
    runAffineEffectiveKappaAnisoNoOpTest();
    runLegacyCovarianceKappa2ResponseTest();
    runSharedRTildeTest();
    runGradientResponseSiteOverloadTest();
    runVelocityClippingTest();

    std::cout << "Flow field model tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Flow field model tests failed: " << error.what() << '\n';
    return 1;
  }
}
