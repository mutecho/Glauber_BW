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
    const double expectedDensityRhoRaw = 0.8 * std::pow(densityNormalSample.rTilde, 1.2)
                                         * std::exp(2.0 * a2 * std::cos(2.0 * (densityNormalSample.phiB - medium.participantGeometry.psi2)));
    requireNear(densityNormalSample.rhoRaw, expectedDensityRhoRaw, 1.0e-12, "Affine density-normal kappa2*eps2 rhoRaw formula mismatch.");
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
    runCovarianceEllipseDirectionTest();
    runDensityNormalDirectionTest();
    runDensityNormalCenterFallbackTest();
    runDensityNormalIgnoresKappa2Test();
    runAffineFlowResponseTest();
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
