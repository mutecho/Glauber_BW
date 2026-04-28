#pragma once

#include <vector>

namespace blastwave {

  /**
   * Transverse coordinate in the laboratory x-y plane, with coordinates in fm.
   */
  struct TransversePoint {
    double x = 0.0;
    double y = 0.0;
  };

  /**
   * Weighted transverse support point for density reconstruction. The current
   * Glauber generator uses unit participant weights, while the explicit weight
   * slot lets future initial-condition models change density strength without
   * changing the medium/evolution interfaces.
   */
  struct WeightedTransversePoint {
    double x = 0.0;
    double y = 0.0;
    double weight = 1.0;
  };

  /**
   * Analytic Gaussian point-cloud density field used as the current transverse
   * density representation for both initial and emission-stage medium states.
   */
  struct DensityField {
    std::vector<WeightedTransversePoint> supportPoints;
    double gaussianSigma = 0.5;  // fm
  };

  /**
   * Density and analytic transverse gradient evaluated at one query point.
   */
  struct DensityFieldSample {
    double density = 0.0;
    double gradientX = 0.0;
    double gradientY = 0.0;
  };

  [[nodiscard]] DensityField buildGaussianPointCloudDensityField(const std::vector<WeightedTransversePoint> &points, double gaussianSigma);
  [[nodiscard]] DensityFieldSample evaluateDensityField(const DensityField &field, double x, double y);

}  // namespace blastwave
