#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

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

  void runNormalDirectionTest() {
    const blastwave::FlowEllipseInfo ellipse = blastwave::computeFlowEllipseInfo(makeAxisAlignedEllipse());
    const blastwave::FlowFieldParameters parameters{0.4, 0.0, 1.0};
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(ellipse, 1.0, 0.5, parameters);
    require(sample.betaT > 0.0, "Normal-direction test needs a non-zero flow sample.");

    const double normalization = std::sqrt((1.0 / 4.0) * (1.0 / 4.0) + (0.5 / 1.0) * (0.5 / 1.0));
    const double expectedNormalX = (1.0 / 4.0) / normalization;
    const double expectedNormalY = (0.5 / 1.0) / normalization;
    const double directionX = sample.betaX / sample.betaT;
    const double directionY = sample.betaY / sample.betaT;
    const double dot = directionX * expectedNormalX + directionY * expectedNormalY;
    requireNear(dot, 1.0, 1.0e-9, "Flow direction should follow the ellipse normal when rho2=0.");
  }

  void runVelocityClippingTest() {
    const blastwave::FlowEllipseInfo ellipse = blastwave::computeFlowEllipseInfo(makeAxisAlignedEllipse());
    const blastwave::FlowFieldParameters parameters{20.0, 20.0, 1.0};
    const blastwave::FlowFieldSample sample = blastwave::evaluateFlowField(ellipse, 2.0, 0.0, parameters);
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
    runNormalDirectionTest();
    runVelocityClippingTest();

    std::cout << "Flow field model tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Flow field model tests failed: " << error.what() << '\n';
    return 1;
  }
}
