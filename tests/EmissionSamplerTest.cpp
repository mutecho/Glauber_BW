#include <cmath>
#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "blastwave/EmissionSampler.h"

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
    runDensityFieldSourceAnchorAndReproducibilityTest();

    std::cout << "Emission sampler tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Emission sampler tests failed: " << error.what() << '\n';
    return 1;
  }
}
