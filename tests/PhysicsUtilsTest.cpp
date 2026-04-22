#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "blastwave/PhysicsUtils.h"

namespace {

  constexpr double kTolerance = 1.0e-12;
  constexpr double kPi = 3.14159265358979323846;

  void requireNear(double actual, double expected, double tolerance, const std::string &message) {
    if (std::abs(actual - expected) > tolerance) {
      throw std::runtime_error(message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
    }
  }

  void runCentralityClampTest() {
    requireNear(blastwave::computeCentralityPercent(0.0, 6.62), 0.0, kTolerance, "Centrality should clamp at zero.");
    requireNear(blastwave::computeCentralityPercent(6.62, 6.62), 50.0, kTolerance, "Centrality midpoint mismatch.");
    requireNear(blastwave::computeCentralityPercent(20.0, 6.62), 100.0, kTolerance, "Centrality should clamp at one hundred.");
  }

  void runAzimuthConventionTest() {
    requireNear(blastwave::computeAzimuth(1.0, 0.0), 0.0, kTolerance, "Azimuth along +x should be zero.");
    requireNear(blastwave::computeAzimuth(0.0, 1.0), 0.5 * kPi, kTolerance, "Azimuth along +y mismatch.");
    requireNear(blastwave::computeAzimuth(-1.0, 0.0), kPi, kTolerance, "Azimuth along -x mismatch.");
  }

  void runPseudorapidityFallbackTest() {
    requireNear(blastwave::computePseudorapidity(1.0, 0.0, 0.0), 0.0, kTolerance, "Mid-rapidity pseudorapidity mismatch.");
    requireNear(blastwave::computePseudorapidity(0.0, 0.0, 1.0), 10.0, kTolerance, "Forward fallback mismatch.");
    requireNear(blastwave::computePseudorapidity(0.0, 0.0, -1.0), -10.0, kTolerance, "Backward fallback mismatch.");
  }

  void runEventV2DefinitionTest() {
    requireNear(blastwave::computeSecondHarmonicEventV2(0.0, 0.0, 0), 0.0, kTolerance, "Empty-event v2 should be zero.");
    requireNear(blastwave::computeSecondHarmonicEventV2(2.0, 0.0, 2), 1.0, kTolerance, "Fully aligned event should give v2=1.");
    requireNear(blastwave::computeSecondHarmonicEventV2(0.0, 0.0, 4), 0.0, kTolerance, "Balanced event should give v2=0.");
    requireNear(blastwave::computeSecondHarmonicEventV2(1.0, std::sqrt(3.0), 4), 0.5, kTolerance, "Q-vector magnitude normalization mismatch.");
  }

}  // namespace

int main() {
  try {
    runCentralityClampTest();
    runAzimuthConventionTest();
    runPseudorapidityFallbackTest();
    runEventV2DefinitionTest();
    std::cout << "Physics utility tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Physics utility tests failed: " << error.what() << '\n';
    return 1;
  }
}
