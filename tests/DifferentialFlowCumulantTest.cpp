#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "blastwave/DifferentialFlowCumulant.h"

namespace {

  constexpr double kPi = 3.14159265358979323846;
  constexpr double kTolerance = 1.0e-12;

  blastwave::DifferentialFlowTrack makeTrack(double pt, double phi) {
    return {pt * std::cos(phi), pt * std::sin(phi)};
  }

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

  void runExactToyTest(int harmonic) {
    blastwave::DifferentialFlowCumulant cumulant(harmonic, {0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});
    cumulant.addEvent({makeTrack(0.2, 0.5 * kPi), makeTrack(0.7, 0.5 * kPi)});

    const blastwave::DifferentialFlowCumulantResult result = cumulant.finalize();
    require(result.harmonic == harmonic, "Exact toy harmonic mismatch.");
    requireNear(result.referenceCumulant, 1.0, kTolerance, "Exact toy reference cumulant mismatch.");
    require(result.contributingEvents == 2U, "Exact toy should record two contributing events.");
    require(result.values.size() == 2U && result.errors.size() == 2U, "Exact toy bin count mismatch.");
    requireNear(result.values[0], 1.0, kTolerance, "Exact toy first-bin value mismatch.");
    requireNear(result.values[1], 1.0, kTolerance, "Exact toy second-bin value mismatch.");
    requireNear(result.errors[0], 0.0, kTolerance, "Exact toy first-bin error should vanish.");
    requireNear(result.errors[1], 0.0, kTolerance, "Exact toy second-bin error should vanish.");
  }

  void runSelfCorrelationSubtractionTest(int harmonic) {
    blastwave::DifferentialFlowCumulant cumulant(harmonic, {0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});

    const blastwave::DifferentialFlowCumulantResult result = cumulant.finalize();
    requireNear(result.referenceCumulant, 1.0, kTolerance, "Self-correlation probe reference cumulant mismatch.");
    requireNear(result.values[0], 1.0, kTolerance, "Self-correlation subtraction should keep the single-POI bin at v_n=1, not 2.");
    requireNear(result.values[1], 1.0, kTolerance, "Reference-aligned partner bin should also yield v_n=1.");
  }

  void runEmptyBinBehaviorTest(int harmonic) {
    blastwave::DifferentialFlowCumulant cumulant(harmonic, {0.0, 0.5, 1.0, 2.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});

    const blastwave::DifferentialFlowCumulantResult result = cumulant.finalize();
    require(result.values.size() == 3U, "Empty-bin test should produce three bins.");
    requireNear(result.values[2], 0.0, kTolerance, "Unfilled tail bin should have zero content.");
    requireNear(result.errors[2], 0.0, kTolerance, "Unfilled tail bin should have zero error.");
  }

  void runMultiplicitySkipTest(int harmonic) {
    blastwave::DifferentialFlowCumulant cumulant(harmonic, {0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0)});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});

    const blastwave::DifferentialFlowCumulantResult result = cumulant.finalize();
    require(result.contributingEvents == 1U, "Only multiplicity>=2 events should contribute.");
    requireNear(result.referenceCumulant, 1.0, kTolerance, "Skipped single-track event must not alter c_n{2}.");
    requireNear(result.values[0], 1.0, kTolerance, "Skipped single-track event must not alter bin 0.");
    requireNear(result.values[1], 1.0, kTolerance, "Skipped single-track event must not alter bin 1.");
  }

  void runNonPositiveReferenceFailureTest(int harmonic) {
    blastwave::DifferentialFlowCumulant cumulant(harmonic, {0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, kPi / static_cast<double>(harmonic))});

    bool threw = false;
    try {
      static_cast<void>(cumulant.finalize());
    } catch (const std::runtime_error &) {
      threw = true;
    }
    require(threw, "Non-positive reference cumulant should fail finalization.");
  }

  void runJackknifeUncertaintyTest(int harmonic) {
    blastwave::DifferentialFlowCumulant cumulant(harmonic, {0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, kPi / (2.0 * static_cast<double>(harmonic)))});

    const blastwave::DifferentialFlowCumulantResult result = cumulant.finalize();
    const double expectedValue = std::sqrt(2.0 / 3.0);
    const double expectedError = 0.19526214587563503;
    requireNear(result.referenceCumulant, 2.0 / 3.0, kTolerance, "Jackknife probe reference cumulant mismatch.");
    require(result.contributingEvents == 3U, "Jackknife probe should record three contributing events.");
    requireNear(result.values[0], expectedValue, kTolerance, "Jackknife first-bin value mismatch.");
    requireNear(result.values[1], expectedValue, kTolerance, "Jackknife second-bin value mismatch.");
    requireNear(result.errors[0], expectedError, kTolerance, "Jackknife first-bin error mismatch.");
    requireNear(result.errors[1], expectedError, kTolerance, "Jackknife second-bin error mismatch.");
  }

  void runInvalidHarmonicRejectTest() {
    bool threw = false;
    try {
      blastwave::DifferentialFlowCumulant cumulant(4, {0.0, 0.5});
      static_cast<void>(cumulant);
    } catch (const std::invalid_argument &) {
      threw = true;
    }
    require(threw, "Only n=2 and n=3 differential-flow cumulants should be accepted.");
  }

  void runHarmonicSuite(int harmonic) {
    runExactToyTest(harmonic);
    runSelfCorrelationSubtractionTest(harmonic);
    runEmptyBinBehaviorTest(harmonic);
    runMultiplicitySkipTest(harmonic);
    runNonPositiveReferenceFailureTest(harmonic);
    runJackknifeUncertaintyTest(harmonic);
  }

}  // namespace

int main() {
  try {
    runHarmonicSuite(2);
    runHarmonicSuite(3);
    runInvalidHarmonicRejectTest();
    std::cout << "Differential-flow cumulant tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Differential-flow cumulant tests failed: " << error.what() << '\n';
    return 1;
  }
}
