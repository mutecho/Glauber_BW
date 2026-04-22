#pragma once

#include <cstddef>
#include <vector>

namespace blastwave {

  // Precomputed inverse-CDF sampler for the isotropic Maxwell-Juttner momentum
  // magnitude shape w(p) = p^2 exp(-sqrt(p^2 + m^2) / T).
  class MaxwellJuttnerMomentumSampler {
   public:
    MaxwellJuttnerMomentumSampler(double mass, double temperature, double pMax, std::size_t gridPoints);

    [[nodiscard]] double sample(double unitUniform) const;

    [[nodiscard]] double mass() const noexcept {
      return mass_;
    }
    [[nodiscard]] double temperature() const noexcept {
      return temperature_;
    }
    [[nodiscard]] double pMax() const noexcept {
      return pMax_;
    }
    [[nodiscard]] std::size_t gridPoints() const noexcept {
      return momentumGrid_.size();
    }
    [[nodiscard]] const std::vector<double> &momentumGrid() const noexcept {
      return momentumGrid_;
    }
    [[nodiscard]] const std::vector<double> &cdf() const noexcept {
      return cdf_;
    }

   private:
    [[nodiscard]] static double thermalWeight(double momentum, double mass, double temperature);

    double mass_ = 0.0;
    double temperature_ = 0.0;
    double pMax_ = 0.0;
    std::vector<double> momentumGrid_;
    std::vector<double> cdf_;
  };

}  // namespace blastwave
