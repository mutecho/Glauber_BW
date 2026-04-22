#pragma once

#include <vector>

namespace blastwave {

  /**
   * Lightweight transverse participant representation used by the ROOT-free
   * flow-field model. The first implementation keeps unit weights in the
   * generator and leaves the explicit weight slot for future extensions.
   */
  struct WeightedTransversePoint {
    double x = 0.0;
    double y = 0.0;
    double weight = 1.0;
  };

  /**
   * Event-level covariance ellipse recovered from the participant cloud.
   * The axis vectors form a deterministic right-handed orthonormal basis in
   * the transverse plane so downstream tests and debug output stay stable.
   */
  struct FlowEllipseInfo {
    bool valid = false;
    double centerX = 0.0;
    double centerY = 0.0;
    double sigmaX2 = 0.0;
    double sigmaY2 = 0.0;
    double sigmaXY = 0.0;
    double lambdaMajor = 0.0;
    double lambdaMinor = 0.0;
    double radiusMajor = 0.0;
    double radiusMinor = 0.0;
    double majorAxisX = 1.0;
    double majorAxisY = 0.0;
    double minorAxisX = 0.0;
    double minorAxisY = 1.0;
    double eps2 = 0.0;
    double psi2 = 0.0;
  };

  /**
   * Public parameters for the covariance-ellipse blast-wave flow profile.
   */
  struct FlowFieldParameters {
    double rho0 = 0.0;
    double rho2 = 0.0;
    double flowPower = 1.0;
  };

  /**
   * ROOT-free flow evaluation result at one emission point in the transverse
   * plane. betaX/betaY live in the transverse laboratory basis before the
   * Bjorken longitudinal composition used by the generator.
   */
  struct FlowFieldSample {
    double betaX = 0.0;
    double betaY = 0.0;
    double betaT = 0.0;
    double rhoRaw = 0.0;
    double rTilde = 0.0;
    double phiB = 0.0;
  };

  [[nodiscard]] FlowEllipseInfo computeFlowEllipseInfo(const std::vector<WeightedTransversePoint> &points);
  [[nodiscard]] FlowFieldSample evaluateFlowField(const FlowEllipseInfo &ellipse, double x, double y, const FlowFieldParameters &parameters);

}  // namespace blastwave
