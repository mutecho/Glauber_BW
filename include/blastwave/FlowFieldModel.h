#pragma once

#include <vector>

#include "blastwave/DensityFieldModel.h"

namespace blastwave {

  enum class FlowVelocitySamplerMode { CovarianceEllipse, DensityNormal };
  struct EventMedium;

  /**
   * Event-level covariance ellipse recovered from the participant cloud.
   * The axis vectors form a deterministic right-handed orthonormal basis in
   * the transverse plane so tests and debug payloads stay stable.
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
    double inverseSigmaXX = 0.0;
    double inverseSigmaXY = 0.0;
    double inverseSigmaYY = 0.0;
    double eps2 = 0.0;
    double psi2 = 0.0;
  };

  /**
   * Public parameters controlling the fluid-element velocity sampler. kappa2 is
   * a response coefficient; the event-wise second-order amplitude is recovered
   * as kappa2 times the initial participant eccentricity.
   */
  struct FlowFieldParameters {
    FlowVelocitySamplerMode velocitySamplerMode = FlowVelocitySamplerMode::CovarianceEllipse;
    double rho0 = 0.0;
    double kappa2 = 0.0;
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
  [[nodiscard]] FlowFieldSample evaluateFlowField(const EventMedium &medium, double x, double y, const FlowFieldParameters &parameters);

}  // namespace blastwave
