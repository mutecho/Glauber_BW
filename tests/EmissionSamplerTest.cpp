#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "blastwave/EmissionSampler.h"
#include "blastwave/PhysicsUtils.h"

namespace {

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

  blastwave::EventMedium makeMedium() {
    return blastwave::buildEventMedium({{-1.0, 0.0, 1.0}, {1.0, 0.0, 1.0}}, {blastwave::DensityEvolutionMode::None, 0.5});
  }

  blastwave::EventMedium makeAffineMedium() {
    return blastwave::buildEventMedium(
        {{-1.0, 0.0, 1.0}, {1.0, 0.0, 1.0}}, {blastwave::DensityEvolutionMode::AffineGaussianResponse, 0.5, 1.20, 1.05, 0.5});
  }

  blastwave::EventMedium makeGradientResponseMedium() {
    return blastwave::buildEventMedium(
        {{-2.0, -1.0, 1.0}, {-2.0, 1.0, 1.0}, {2.0, -1.0, 1.0}, {2.0, 1.0, 1.0}},
        {blastwave::DensityEvolutionMode::GradientResponse, 0.5, 1.20, 1.05, 0.5, 0.0, 1.0, 1.0e-4, 1.0e-6});
  }

  blastwave::EmissionParameters makeGradientResponseParameters(double gradientDisplacementMax,
                                                               double gradientDiffusionSigma,
                                                               double gradientVMax,
                                                               blastwave::CooperFryeWeightMode cooperFryeWeightMode = blastwave::CooperFryeWeightMode::None) {
    return {blastwave::EmissionSamplerMode::GradientResponse,
            0.5,
            30.0,
            20.0,
            1.0e-4,
            1.0e-6,
            gradientDisplacementMax,
            1.0,
            gradientDiffusionSigma,
            gradientVMax,
            1.0,
            cooperFryeWeightMode};
  }

  void requireSitesEqual(const blastwave::EmissionSite &actual, const blastwave::EmissionSite &expected, double tolerance, const std::string &messagePrefix) {
    requireNear(actual.initialPosition.x, expected.initialPosition.x, tolerance, messagePrefix + " initialPosition.x mismatch.");
    requireNear(actual.initialPosition.y, expected.initialPosition.y, tolerance, messagePrefix + " initialPosition.y mismatch.");
    requireNear(actual.position.x, expected.position.x, tolerance, messagePrefix + " position.x mismatch.");
    requireNear(actual.position.y, expected.position.y, tolerance, messagePrefix + " position.y mismatch.");
    requireNear(actual.sourceAnchor.x, expected.sourceAnchor.x, tolerance, messagePrefix + " sourceAnchor.x mismatch.");
    requireNear(actual.sourceAnchor.y, expected.sourceAnchor.y, tolerance, messagePrefix + " sourceAnchor.y mismatch.");
    requireNear(actual.gradientMagnitude, expected.gradientMagnitude, tolerance, messagePrefix + " gradientMagnitude mismatch.");
    requireNear(actual.displacementX, expected.displacementX, tolerance, messagePrefix + " displacementX mismatch.");
    requireNear(actual.displacementY, expected.displacementY, tolerance, messagePrefix + " displacementY mismatch.");
    requireNear(actual.betaTX, expected.betaTX, tolerance, messagePrefix + " betaTX mismatch.");
    requireNear(actual.betaTY, expected.betaTY, tolerance, messagePrefix + " betaTY mismatch.");
    requireNear(actual.emissionWeight, expected.emissionWeight, tolerance, messagePrefix + " emissionWeight mismatch.");
  }

  void runZeroMultiplicityTest() {
    std::mt19937_64 rng(12345);
    const blastwave::EmissionParameters parameters{blastwave::EmissionSamplerMode::ParticipantHotspot, 0.5, 0.0, 1.5};
    const std::vector<blastwave::EmissionSite> sites = blastwave::sampleEmissionSites(makeMedium(), parameters, rng);
    require(sites.empty(), "Participant-hotspot emission should return no sites when nbdMu is disabled.");
  }

  void runNoSmearKeepsAnchorTest() {
    std::mt19937_64 rng(12345);
    const blastwave::EmissionParameters parameters{blastwave::EmissionSamplerMode::ParticipantHotspot, 0.0, 100.0, 100.0};
    const std::vector<blastwave::EmissionSite> sites = blastwave::sampleEmissionSites(makeMedium(), parameters, rng);
    require(!sites.empty(), "No-smear test needs at least one emitted site.");

    for (const blastwave::EmissionSite &site : sites) {
      requireNear(site.position.x, site.sourceAnchor.x, 1.0e-12, "No-smear emission x should equal source anchor x.");
      requireNear(site.position.y, site.sourceAnchor.y, 1.0e-12, "No-smear emission y should equal source anchor y.");
      require(std::abs(site.sourceAnchor.x + 1.0) < 1.0e-12 || std::abs(site.sourceAnchor.x - 1.0) < 1.0e-12,
              "sourceAnchor.x should preserve a participant coordinate.");
      requireNear(site.sourceAnchor.y, 0.0, 1.0e-12, "sourceAnchor.y should preserve the participant coordinate.");
    }
  }

  void runReproducibilityTest() {
    const blastwave::EmissionParameters parameters{blastwave::EmissionSamplerMode::ParticipantHotspot, 0.5, 10.0, 3.0};
    std::mt19937_64 rngA(98765);
    std::mt19937_64 rngB(98765);
    const std::vector<blastwave::EmissionSite> sitesA = blastwave::sampleEmissionSites(makeMedium(), parameters, rngA);
    const std::vector<blastwave::EmissionSite> sitesB = blastwave::sampleEmissionSites(makeMedium(), parameters, rngB);

    require(sitesA.size() == sitesB.size(), "Emission sampler should be reproducible for a fixed RNG seed.");
    for (std::size_t iSite = 0; iSite < sitesA.size(); ++iSite) {
      requireNear(sitesA[iSite].position.x, sitesB[iSite].position.x, 1.0e-12, "Reproducible emission position.x mismatch.");
      requireNear(sitesA[iSite].position.y, sitesB[iSite].position.y, 1.0e-12, "Reproducible emission position.y mismatch.");
      requireNear(sitesA[iSite].sourceAnchor.x, sitesB[iSite].sourceAnchor.x, 1.0e-12, "Reproducible sourceAnchor.x mismatch.");
      requireNear(sitesA[iSite].sourceAnchor.y, sitesB[iSite].sourceAnchor.y, 1.0e-12, "Reproducible sourceAnchor.y mismatch.");
    }
  }

  void runGradientResponseReproducibilityTest() {
    const blastwave::EventMedium medium = makeGradientResponseMedium();
    const blastwave::EmissionParameters parameters = makeGradientResponseParameters(1.5, 0.0, 0.75, blastwave::CooperFryeWeightMode::MtCosh);
    std::mt19937_64 rngA(112233);
    std::mt19937_64 rngB(112233);
    const std::vector<blastwave::EmissionSite> sitesA = blastwave::sampleEmissionSites(medium, parameters, rngA);
    const std::vector<blastwave::EmissionSite> sitesB = blastwave::sampleEmissionSites(medium, parameters, rngB);

    require(sitesA.size() == sitesB.size(), "Gradient-response emission should be reproducible for a fixed RNG seed.");
    require(!sitesA.empty(), "Gradient-response reproducibility test needs non-empty output.");
    for (std::size_t iSite = 0; iSite < sitesA.size(); ++iSite) {
      requireSitesEqual(sitesA[iSite], sitesB[iSite], 1.0e-12, "Gradient-response reproducible site");
      require(std::isfinite(sitesA[iSite].emissionWeight) && sitesA[iSite].emissionWeight >= 0.0,
              "Gradient-response emission weights should stay finite and non-negative.");
    }
  }

  void runGradientResponseIdentityTest() {
    const blastwave::EventMedium medium = makeGradientResponseMedium();
    const blastwave::EmissionParameters parameters = makeGradientResponseParameters(0.0, 0.0, 0.75);
    std::mt19937_64 rng(445566);
    const std::vector<blastwave::EmissionSite> sites = blastwave::sampleEmissionSites(medium, parameters, rng);

    require(!sites.empty(), "Identity test needs at least one gradient-response site.");
    for (const blastwave::EmissionSite &site : sites) {
      requireNear(site.position.x, site.initialPosition.x, 1.0e-12, "Zero gradient displacement should keep x unchanged.");
      requireNear(site.position.y, site.initialPosition.y, 1.0e-12, "Zero gradient displacement should keep y unchanged.");
      requireNear(site.displacementX, 0.0, 1.0e-12, "Zero gradient displacement should produce zero displacementX.");
      requireNear(site.displacementY, 0.0, 1.0e-12, "Zero gradient displacement should produce zero displacementY.");
      require(std::isfinite(site.emissionWeight) && site.emissionWeight >= 0.0, "Emission weight should stay finite and non-negative.");
    }
  }

  void runGradientResponseZeroBetaTest() {
    const blastwave::EventMedium medium = makeGradientResponseMedium();
    const blastwave::EmissionParameters parameters = makeGradientResponseParameters(1.5, 0.0, 0.0);
    std::mt19937_64 rng(778899);
    const std::vector<blastwave::EmissionSite> sites = blastwave::sampleEmissionSites(medium, parameters, rng);

    require(!sites.empty(), "Zero-beta test needs at least one gradient-response site.");
    for (const blastwave::EmissionSite &site : sites) {
      requireNear(site.betaTX, 0.0, 1.0e-12, "gradientVMax=0 should produce zero betaTX.");
      requireNear(site.betaTY, 0.0, 1.0e-12, "gradientVMax=0 should produce zero betaTY.");
      require(std::isfinite(site.emissionWeight) && site.emissionWeight >= 0.0, "Emission weight should stay finite and non-negative.");
    }
  }

  void runGradientResponseDisplacementBoundAndAnchorTest() {
    const blastwave::EventMedium medium = makeGradientResponseMedium();
    const blastwave::EmissionParameters parameters = makeGradientResponseParameters(1.5, 0.0, 0.75);
    std::mt19937_64 rng(991122);
    const std::vector<blastwave::EmissionSite> sites = blastwave::sampleEmissionSites(medium, parameters, rng);

    require(!sites.empty(), "Displacement-bound test needs at least one gradient-response site.");
    bool foundInitialShift = false;
    for (const blastwave::EmissionSite &site : sites) {
      const double displacementLength = std::hypot(site.displacementX, site.displacementY);
      require(displacementLength <= 1.5 + 1.0e-12, "Diffusion-free gradient displacement should not exceed Lmax.");
      require(std::isfinite(site.emissionWeight) && site.emissionWeight >= 0.0, "Emission weight should stay finite and non-negative.");
      if (std::abs(site.initialPosition.x - site.sourceAnchor.x) > 1.0e-8 || std::abs(site.initialPosition.y - site.sourceAnchor.y) > 1.0e-8) {
        foundInitialShift = true;
      }
    }
    require(foundInitialShift, "At least one gradient-response site should shift from the participant anchor to the marker initial position.");
  }

  void runGradientResponseMeanR2ExpansionTest() {
    const blastwave::EventMedium medium = makeGradientResponseMedium();
    const blastwave::EmissionParameters parameters = makeGradientResponseParameters(1.5, 0.0, 0.75);
    std::mt19937_64 rng(223344);
    const std::vector<blastwave::EmissionSite> sites = blastwave::sampleEmissionSites(medium, parameters, rng);

    require(!sites.empty(), "Mean-r2 test needs at least one gradient-response site.");
    std::vector<blastwave::TransversePoint> initialPoints;
    std::vector<blastwave::TransversePoint> finalPoints;
    initialPoints.reserve(sites.size());
    finalPoints.reserve(sites.size());
    for (const blastwave::EmissionSite &site : sites) {
      initialPoints.push_back(site.initialPosition);
      finalPoints.push_back(site.position);
    }

    const double meanR2Initial = blastwave::computeMeanRadiusSquared(initialPoints);
    const double meanR2Final = blastwave::computeMeanRadiusSquared(finalPoints);
    require(meanR2Final > meanR2Initial, "Gradient displacement should expand the mean r^2 for this deterministic ellipse probe.");
  }

  void runDensityFieldSourceAnchorAndReproducibilityTest() {
    const blastwave::EventMedium medium = makeAffineMedium();
    const blastwave::EmissionParameters parameters{blastwave::EmissionSamplerMode::DensityField, 0.5, 20.0, 5.0};
    std::mt19937_64 rngA(24680);
    std::mt19937_64 rngB(24680);
    const std::vector<blastwave::EmissionSite> sitesA = blastwave::sampleEmissionSites(medium, parameters, rngA);
    const std::vector<blastwave::EmissionSite> sitesB = blastwave::sampleEmissionSites(medium, parameters, rngB);

    require(sitesA.size() == sitesB.size(), "Density-field emission should be reproducible for a fixed RNG seed.");
    require(!sitesA.empty(), "Density-field emission test needs non-empty output.");
    bool foundNonTrivialShift = false;
    for (std::size_t iSite = 0; iSite < sitesA.size(); ++iSite) {
      requireNear(sitesA[iSite].position.x, sitesB[iSite].position.x, 1.0e-12, "Density-field reproducible position.x mismatch.");
      requireNear(sitesA[iSite].position.y, sitesB[iSite].position.y, 1.0e-12, "Density-field reproducible position.y mismatch.");
      requireNear(sitesA[iSite].sourceAnchor.x, sitesB[iSite].sourceAnchor.x, 1.0e-12, "Density-field reproducible sourceAnchor.x mismatch.");
      requireNear(sitesA[iSite].sourceAnchor.y, sitesB[iSite].sourceAnchor.y, 1.0e-12, "Density-field reproducible sourceAnchor.y mismatch.");
      require(std::abs(sitesA[iSite].sourceAnchor.x + 1.0) < 1.0e-12 || std::abs(sitesA[iSite].sourceAnchor.x - 1.0) < 1.0e-12,
              "Density-field sourceAnchor.x should preserve original participant anchors.");
      requireNear(sitesA[iSite].sourceAnchor.y, 0.0, 1.0e-12, "Density-field sourceAnchor.y should preserve original participant anchors.");
      if (std::abs(sitesA[iSite].position.x - sitesA[iSite].sourceAnchor.x) > 1.0e-8 || std::abs(sitesA[iSite].position.y - sitesA[iSite].sourceAnchor.y) > 1.0e-8) {
        foundNonTrivialShift = true;
      }
    }
    require(foundNonTrivialShift, "Density-field emission positions should reflect freeze-out sampling rather than always equal source anchors.");
  }

}  // namespace

int main() {
  try {
    runZeroMultiplicityTest();
    runNoSmearKeepsAnchorTest();
    runReproducibilityTest();
    runGradientResponseReproducibilityTest();
    runGradientResponseIdentityTest();
    runGradientResponseZeroBetaTest();
    runGradientResponseDisplacementBoundAndAnchorTest();
    runGradientResponseMeanR2ExpansionTest();
    runDensityFieldSourceAnchorAndReproducibilityTest();

    std::cout << "Emission sampler tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Emission sampler tests failed: " << error.what() << '\n';
    return 1;
  }
}
