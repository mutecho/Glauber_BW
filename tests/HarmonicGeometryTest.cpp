#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "blastwave/PhysicsUtils.h"

namespace {

  constexpr double kTolerance = 1.0e-12;
  constexpr double kPi = 3.14159265358979323846;

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

  bool isAngleEquivalent(double observed, double expected, double tolerance) {
    const double wrappedObserved = std::fmod(std::fmod(observed - expected + kPi, 2.0 * kPi) + 2.0 * kPi, 2.0 * kPi) - kPi;
    return std::abs(wrappedObserved) < tolerance;
  }

  void runInvalidHarmonicRejectsTest() {
    const auto result = blastwave::computeRecenteredHarmonicGeometry({}, 0);
    require(!result.valid, "Harmonic 0 must be rejected by geometry helper.");
  }

  void runTriangleGeometryTest() {
    const std::vector<blastwave::WeightedTransversePoint> points{{1.0, 0.0, 1.0}, {-0.5, std::sqrt(3.0) / 2.0, 1.0}, {-0.5, -std::sqrt(3.0) / 2.0, 1.0}};
    const auto result = blastwave::computeRecenteredHarmonicGeometry(points, 3);
    require(result.valid, "Triangle harmonic helper should return valid result.");
    requireNear(result.epsilon, 1.0, 1.0e-12, "Equilateral triangle should have unit epsilon3.");
    require(isAngleEquivalent(result.psi, kPi / 3.0, 1.0e-12) || isAngleEquivalent(result.psi, -kPi / 3.0, 1.0e-12), "Triangle psi3 should match phase convention up to sign.");
    require(std::abs(result.centerX) < kTolerance, "Triangle helper should not shift center x on symmetric input.");
    require(std::abs(result.centerY) < kTolerance, "Triangle helper should not shift center y on symmetric input.");
    requireNear(result.rRms, 1.0, 1.0e-12, "Triangle helper should keep unit radius RMS.");
  }

  void runCircleReturnsTinyEps3Test() {
    std::vector<blastwave::WeightedTransversePoint> points;
    for (int iPoint = 0; iPoint < 12; ++iPoint) {
      const double phi = 2.0 * kPi * static_cast<double>(iPoint) / 12.0;
      points.push_back({std::cos(phi), std::sin(phi), 1.0});
    }
    const auto result = blastwave::computeRecenteredHarmonicGeometry(points, 3);
    require(result.valid, "Circle harmonic helper should be valid for finite points.");
    require(std::abs(result.epsilon) < 1.0e-10, "Circle-like source should have tiny epsilon3.");
  }

  void runShiftInvarianceTest() {
    std::vector<blastwave::WeightedTransversePoint> shifted{{2.0, -1.0, 1.0}, {3.0, 1.0, 1.0}, {1.0, 2.0, 1.0}};
    const auto result = blastwave::computeRecenteredHarmonicGeometry(shifted, 3);
    require(result.valid, "Shifted geometry should remain finite.");
    const double centerDistance = std::hypot(result.centerX - 2.0, result.centerY - 0.6666666666666666);
    require(centerDistance < 1.0e-12, "Recenter helper should return the actual source centroid.");
  }

}  // namespace

int main() {
  try {
    runInvalidHarmonicRejectsTest();
    runTriangleGeometryTest();
    runCircleReturnsTinyEps3Test();
    runShiftInvarianceTest();
    std::cout << "Harmonic geometry tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Harmonic geometry tests failed: " << error.what() << '\n';
    return 1;
  }
}
