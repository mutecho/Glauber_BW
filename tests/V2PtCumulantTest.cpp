#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "blastwave/V2PtCumulant.h"

namespace {

  constexpr double kPi = 3.14159265358979323846;
  constexpr double kTolerance = 1.0e-12;

  blastwave::V2PtTrack makeTrack(double pt, double phi) {
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

  void runExactToyTest() {
    blastwave::V2PtCumulant cumulant({0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});
    cumulant.addEvent({makeTrack(0.2, 0.5 * kPi), makeTrack(0.7, 0.5 * kPi)});

    const blastwave::V2PtCumulantResult result = cumulant.finalize();
    requireNear(result.c2, 1.0, kTolerance, "Exact toy c2 mismatch.");
    require(result.contributingEvents == 2U, "Exact toy should record two contributing events.");
    require(result.v2Values.size() == 2U && result.v2Errors.size() == 2U, "Exact toy bin count mismatch.");
    requireNear(result.v2Values[0], 1.0, kTolerance, "Exact toy first-bin v2 mismatch.");
    requireNear(result.v2Values[1], 1.0, kTolerance, "Exact toy second-bin v2 mismatch.");
    requireNear(result.v2Errors[0], 0.0, kTolerance, "Exact toy first-bin error should vanish.");
    requireNear(result.v2Errors[1], 0.0, kTolerance, "Exact toy second-bin error should vanish.");
  }

  void runSelfCorrelationSubtractionTest() {
    blastwave::V2PtCumulant cumulant({0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});

    const blastwave::V2PtCumulantResult result = cumulant.finalize();
    requireNear(result.c2, 1.0, kTolerance, "Self-correlation probe c2 mismatch.");
    requireNear(result.v2Values[0], 1.0, kTolerance, "Self-correlation subtraction should keep the single-POI bin at v2=1, not 2.");
    requireNear(result.v2Values[1], 1.0, kTolerance, "Reference-aligned partner bin should also yield v2=1.");
  }

  void runEmptyBinBehaviorTest() {
    blastwave::V2PtCumulant cumulant({0.0, 0.5, 1.0, 2.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});

    const blastwave::V2PtCumulantResult result = cumulant.finalize();
    require(result.v2Values.size() == 3U, "Empty-bin test should produce three bins.");
    requireNear(result.v2Values[2], 0.0, kTolerance, "Unfilled tail bin should have zero content.");
    requireNear(result.v2Errors[2], 0.0, kTolerance, "Unfilled tail bin should have zero error.");
  }

  void runMultiplicitySkipTest() {
    blastwave::V2PtCumulant cumulant({0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0)});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});

    const blastwave::V2PtCumulantResult result = cumulant.finalize();
    require(result.contributingEvents == 1U, "Only multiplicity>=2 events should contribute.");
    requireNear(result.c2, 1.0, kTolerance, "Skipped single-track event must not alter c2.");
    requireNear(result.v2Values[0], 1.0, kTolerance, "Skipped single-track event must not alter bin 0.");
    requireNear(result.v2Values[1], 1.0, kTolerance, "Skipped single-track event must not alter bin 1.");
  }

  void runNonPositiveC2FailureTest() {
    blastwave::V2PtCumulant cumulant({0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.5 * kPi)});

    bool threw = false;
    try {
      static_cast<void>(cumulant.finalize());
    } catch (const std::runtime_error &) {
      threw = true;
    }
    require(threw, "Non-positive c2 should fail finalization.");
  }

  void runJackknifeUncertaintyTest() {
    blastwave::V2PtCumulant cumulant({0.0, 0.5, 1.0});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.0)});
    cumulant.addEvent({makeTrack(0.2, 0.0), makeTrack(0.7, 0.25 * kPi)});

    const blastwave::V2PtCumulantResult result = cumulant.finalize();
    const double expectedValue = std::sqrt(2.0 / 3.0);
    const double expectedError = 0.19526214587563503;
    requireNear(result.c2, 2.0 / 3.0, kTolerance, "Jackknife probe c2 mismatch.");
    require(result.contributingEvents == 3U, "Jackknife probe should record three contributing events.");
    requireNear(result.v2Values[0], expectedValue, kTolerance, "Jackknife first-bin value mismatch.");
    requireNear(result.v2Values[1], expectedValue, kTolerance, "Jackknife second-bin value mismatch.");
    requireNear(result.v2Errors[0], expectedError, kTolerance, "Jackknife first-bin error mismatch.");
    requireNear(result.v2Errors[1], expectedError, kTolerance, "Jackknife second-bin error mismatch.");
  }

}  // namespace

int main() {
  try {
    runExactToyTest();
    runSelfCorrelationSubtractionTest();
    runEmptyBinBehaviorTest();
    runMultiplicitySkipTest();
    runNonPositiveC2FailureTest();
    runJackknifeUncertaintyTest();
    std::cout << "V2Pt cumulant tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "V2Pt cumulant tests failed: " << error.what() << '\n';
    return 1;
  }
}
