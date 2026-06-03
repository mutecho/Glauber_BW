#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "blastwave/BlastWaveGenerator.h"

namespace {

  constexpr double kTolerance = 1.0e-12;

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

  // Build the fixed response-test baseline used as a closure regression.
  blastwave::BlastWaveConfig makeFixedResponseConfig() {
    blastwave::BlastWaveConfig config;
    config.initialGeometryMode = blastwave::InitialGeometryMode::ResponseTest023;
    config.initialGeometrySourceCount = 120;
    config.initialGeometryA2 = 0.4;
    config.initialGeometryA3 = 0.6;
    config.initialGeometryR2x = 2.1;
    config.initialGeometryR2y = 1.2;
    config.initialGeometryR3 = 2.0;
    config.initialGeometrySigma3 = 0.45;
    config.densityEvolutionMode = blastwave::DensityEvolutionMode::None;
    config.flowVelocitySamplerMode = blastwave::FlowVelocitySamplerMode::DensityNormal;
    config.seed = 12345;
    config.nEvents = 1;
    return config;
  }

  // Build a stochastic response-test config with narrow enough ranges for fast tests.
  blastwave::BlastWaveConfig makeFluctuatingResponseConfig() {
    blastwave::BlastWaveConfig config = makeFixedResponseConfig();
    config.initialGeometryFluctuate = true;
    config.initialGeometrySourceCountMin = 17;
    config.initialGeometrySourceCountMax = 29;
    config.initialGeometryA2Min = 0.1;
    config.initialGeometryA2Max = 0.9;
    config.initialGeometryA3Min = 0.2;
    config.initialGeometryA3Max = 0.6;
    config.initialGeometryR2xMin = 1.8;
    config.initialGeometryR2xMax = 3.2;
    config.initialGeometryR2yMin = 0.9;
    config.initialGeometryR2yMax = 1.8;
    config.initialGeometryR3Min = 1.4;
    config.initialGeometryR3Max = 3.0;
    config.initialGeometrySigma3Min = 0.45;
    config.initialGeometrySigma3Max = 1.3;
    config.hasInitialGeometrySourceCountMin = true;
    config.hasInitialGeometrySourceCountMax = true;
    config.hasInitialGeometryA2Min = true;
    config.hasInitialGeometryA2Max = true;
    config.hasInitialGeometryA3Min = true;
    config.hasInitialGeometryA3Max = true;
    config.hasInitialGeometryR2xMin = true;
    config.hasInitialGeometryR2xMax = true;
    config.hasInitialGeometryR2yMin = true;
    config.hasInitialGeometryR2yMax = true;
    config.hasInitialGeometryR3Min = true;
    config.hasInitialGeometryR3Max = true;
    config.hasInitialGeometrySigma3Min = true;
    config.hasInitialGeometrySigma3Max = true;
    return config;
  }

  // Assert one event's sampled template metadata is inside the configured ranges.
  void requireFluctuatingEventInRange(const blastwave::BlastWaveConfig &config, const blastwave::GeneratedEvent &event) {
    require(event.info.nParticipants >= config.initialGeometrySourceCountMin && event.info.nParticipants <= config.initialGeometrySourceCountMax,
            "Fluctuating response Npart should be inside source-count range.");
    require(event.participants.size() == static_cast<std::size_t>(event.info.nParticipants),
            "Fluctuating response participant vector should match EventInfo Npart.");
    require(event.info.geoA2 >= config.initialGeometryA2Min && event.info.geoA2 <= config.initialGeometryA2Max,
            "Fluctuating geoA2 should be inside configured range.");
    require(event.info.geoA3 >= config.initialGeometryA3Min && event.info.geoA3 <= config.initialGeometryA3Max,
            "Fluctuating geoA3 should be inside configured range.");
    require(event.info.geoR2x >= config.initialGeometryR2xMin && event.info.geoR2x <= config.initialGeometryR2xMax,
            "Fluctuating geoR2x should be inside configured range.");
    require(event.info.geoR2y >= config.initialGeometryR2yMin && event.info.geoR2y <= config.initialGeometryR2yMax,
            "Fluctuating geoR2y should be inside configured range.");
    require(event.info.geoR3 >= config.initialGeometryR3Min && event.info.geoR3 <= config.initialGeometryR3Max,
            "Fluctuating geoR3 should be inside configured range.");
    require(event.info.geoSigma3 >= config.initialGeometrySigma3Min && event.info.geoSigma3 <= config.initialGeometrySigma3Max,
            "Fluctuating geoSigma3 should be inside configured range.");
  }

  // Compare the stochastic metadata sequence from two generators with identical seeds.
  void requireSameTemplateMetadata(const blastwave::GeneratedEvent &lhs, const blastwave::GeneratedEvent &rhs) {
    require(lhs.info.nParticipants == rhs.info.nParticipants, "Same seed should reproduce fluctuating Npart.");
    requireNear(lhs.info.geoA2, rhs.info.geoA2, 0.0, "Same seed should reproduce geoA2.");
    requireNear(lhs.info.geoA3, rhs.info.geoA3, 0.0, "Same seed should reproduce geoA3.");
    requireNear(lhs.info.geoR2x, rhs.info.geoR2x, 0.0, "Same seed should reproduce geoR2x.");
    requireNear(lhs.info.geoR2y, rhs.info.geoR2y, 0.0, "Same seed should reproduce geoR2y.");
    requireNear(lhs.info.geoR3, rhs.info.geoR3, 0.0, "Same seed should reproduce geoR3.");
    requireNear(lhs.info.geoSigma3, rhs.info.geoSigma3, 0.0, "Same seed should reproduce geoSigma3.");
  }

  void runResponseModeTemplateTest() {
    blastwave::BlastWaveConfig config = makeFixedResponseConfig();

    blastwave::BlastWaveGenerator generator(config);
    const blastwave::GeneratedEvent event = generator.generateEvent(0);

    require(event.info.initialGeometryMode == 1, "Response event should encode initial-geometry mode as 1.");
    require(event.participants.size() == static_cast<std::size_t>(config.initialGeometrySourceCount), "Response template should generate requested source count.");
    for (const blastwave::ParticipantRecord &participant : event.participants) {
      require(participant.nucleusId == -1, "Response participants should be tagged with nucleusId=-1.");
    }

    const double centerX = std::abs(event.info.xCenterInitial);
    const double centerY = std::abs(event.info.yCenterInitial);
    require(centerX < 1.0e-12, "Response geometry should be recentred to zero center x.");
    require(centerY < 1.0e-12, "Response geometry should be recentred to zero center y.");

    require(event.info.eps3 >= 0.0 && std::isfinite(event.info.eps3), "eps3 should be finite and non-negative.");
    require(event.info.v3 >= 0.0 && event.info.v3 <= 1.0 + kTolerance, "v3 should lie in [0,1] for finite event sample.");
    require(std::isfinite(event.info.v2WrtPsi2), "v2_wrt_psi2 should be finite.");
    require(std::isfinite(event.info.v3WrtPsi3), "v3_wrt_psi3 should be finite.");
    requireNear(event.info.geoA2, config.initialGeometryA2, kTolerance, "Fixed response geoA2 should mirror config.");
    requireNear(event.info.geoA3, config.initialGeometryA3, kTolerance, "Fixed response geoA3 should mirror config.");
    requireNear(event.info.geoR2x, config.initialGeometryR2x, kTolerance, "Fixed response geoR2x should mirror config.");
    requireNear(event.info.geoR2y, config.initialGeometryR2y, kTolerance, "Fixed response geoR2y should mirror config.");
    requireNear(event.info.geoR3, config.initialGeometryR3, kTolerance, "Fixed response geoR3 should mirror config.");
    requireNear(event.info.geoSigma3, config.initialGeometrySigma3, kTolerance, "Fixed response geoSigma3 should mirror config.");
    require(event.info.nCharged > 0, "Response sample should emit at least one particle.");
  }

  void runResponseModeIndependentPoolsSourceCountTest() {
    blastwave::BlastWaveConfig config = makeFixedResponseConfig();
    config.initialGeometrySourceAllocationMode = blastwave::InitialGeometrySourceAllocationMode::IndependentPools;
    config.initialGeometrySourceCount = 10;
    config.initialGeometryA2 = 0.4;
    config.initialGeometryA3 = 0.3;

    blastwave::BlastWaveGenerator generator(config);
    const blastwave::GeneratedEvent event = generator.generateEvent(0);

    require(event.info.initialGeometryMode == 1, "Independent-pools response event should stay in response-test mode.");
    require(event.info.nParticipants == 17, "Independent pools should use N0 + round(N0*A2) + round(N0*A3).");
    require(event.participants.size() == static_cast<std::size_t>(event.info.nParticipants),
            "Independent-pools participant vector should match EventInfo Npart.");
  }

  void runResponseModeIndependentPoolsTriangularMinimumTest() {
    blastwave::BlastWaveConfig config = makeFixedResponseConfig();
    config.initialGeometrySourceAllocationMode = blastwave::InitialGeometrySourceAllocationMode::IndependentPools;
    config.initialGeometrySourceCount = 10;
    config.initialGeometryA2 = 0.0;
    config.initialGeometryA3 = 0.1;

    blastwave::BlastWaveGenerator generator(config);
    const blastwave::GeneratedEvent event = generator.generateEvent(0);

    require(event.info.nParticipants == 13, "Positive A3 independent pools should provide at least three triangular hotspot sources.");
  }

  // Verify fluctuating response-test events draw source count and metadata per event.
  void runResponseModeFluctuatingRangeTest() {
    const blastwave::BlastWaveConfig config = makeFluctuatingResponseConfig();
    blastwave::BlastWaveGenerator generator(config);

    bool sawDifferentNpart = false;
    bool sawDifferentTemplateParameter = false;
    int firstNpart = -1;
    double firstGeoA2 = -1.0;
    for (int eventId = 0; eventId < 24; ++eventId) {
      const blastwave::GeneratedEvent event = generator.generateEvent(eventId);
      requireFluctuatingEventInRange(config, event);
      if (eventId == 0) {
        firstNpart = event.info.nParticipants;
        firstGeoA2 = event.info.geoA2;
      } else {
        sawDifferentNpart = sawDifferentNpart || event.info.nParticipants != firstNpart;
        sawDifferentTemplateParameter = sawDifferentTemplateParameter || std::abs(event.info.geoA2 - firstGeoA2) > 0.0;
      }
    }

    require(sawDifferentNpart, "Fluctuating response should produce more than one source count across events.");
    require(sawDifferentTemplateParameter, "Fluctuating response should produce more than one template parameter across events.");
  }

  // Verify fixed seed reproducibility for the per-event stochastic template stream.
  void runResponseModeFluctuatingSeedReproducibleTest() {
    const blastwave::BlastWaveConfig config = makeFluctuatingResponseConfig();
    blastwave::BlastWaveGenerator firstGenerator(config);
    blastwave::BlastWaveGenerator secondGenerator(config);

    for (int eventId = 0; eventId < 12; ++eventId) {
      const blastwave::GeneratedEvent firstEvent = firstGenerator.generateEvent(eventId);
      const blastwave::GeneratedEvent secondEvent = secondGenerator.generateEvent(eventId);
      requireFluctuatingEventInRange(config, firstEvent);
      requireSameTemplateMetadata(firstEvent, secondEvent);
    }
  }

}  // namespace

int main() {
  try {
    runResponseModeTemplateTest();
    runResponseModeIndependentPoolsSourceCountTest();
    runResponseModeIndependentPoolsTriangularMinimumTest();
    runResponseModeFluctuatingRangeTest();
    runResponseModeFluctuatingSeedReproducibleTest();
    std::cout << "Blast-wave generator response tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Blast-wave generator response tests failed: " << error.what() << '\n';
    return 1;
  }
}
