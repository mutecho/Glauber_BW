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

  void runResponseModeTemplateTest() {
    blastwave::BlastWaveConfig config;
    config.initialGeometryMode = blastwave::InitialGeometryMode::ResponseTest023;
    config.initialGeometrySourceCount = 120;
    config.initialGeometryA2 = 0.4;
    config.initialGeometryA3 = 0.6;
    config.initialGeometryR3 = 2.0;
    config.initialGeometrySigma3 = 0.45;
    config.densityEvolutionMode = blastwave::DensityEvolutionMode::None;
    config.flowVelocitySamplerMode = blastwave::FlowVelocitySamplerMode::DensityNormal;
    config.seed = 12345;
    config.nEvents = 1;

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
    require(event.info.nCharged > 0, "Response sample should emit at least one particle.");
  }

}  // namespace

int main() {
  try {
    runResponseModeTemplateTest();
    std::cout << "Blast-wave generator response tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Blast-wave generator response tests failed: " << error.what() << '\n';
    return 1;
  }
}
