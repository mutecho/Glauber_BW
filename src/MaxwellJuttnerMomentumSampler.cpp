#include "blastwave/MaxwellJuttnerMomentumSampler.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace blastwave {

  namespace {

    bool isFinite(double value) {
      return std::isfinite(value);
    }

  }  // namespace

  MaxwellJuttnerMomentumSampler::MaxwellJuttnerMomentumSampler(double mass, double temperature, double pMax, std::size_t gridPoints)
      : mass_(mass), temperature_(temperature), pMax_(pMax) {
    if (mass_ <= 0.0) {
      throw std::invalid_argument("Maxwell-Juttner sampler mass must be positive.");
    }
    if (temperature_ <= 0.0) {
      throw std::invalid_argument("Maxwell-Juttner sampler temperature must be positive.");
    }
    if (pMax_ <= 0.0) {
      throw std::invalid_argument("Maxwell-Juttner sampler pMax must be positive.");
    }
    if (gridPoints < 2U) {
      throw std::invalid_argument("Maxwell-Juttner sampler grid must contain at least 2 points.");
    }

    momentumGrid_.resize(gridPoints, 0.0);
    cdf_.resize(gridPoints, 0.0);
    std::vector<double> weights(gridPoints, 0.0);

    const double gridSpacing = pMax_ / static_cast<double>(gridPoints - 1U);
    for (std::size_t index = 0; index < gridPoints; ++index) {
      const double momentumMagnitude = static_cast<double>(index) * gridSpacing;
      momentumGrid_[index] = momentumMagnitude;
      weights[index] = thermalWeight(momentumMagnitude, mass_, temperature_);
      if (!isFinite(weights[index])) {
        throw std::runtime_error("Maxwell-Juttner sampler produced a non-finite table weight.");
      }
    }

    double cumulativeIntegral = 0.0;
    cdf_.front() = 0.0;
    for (std::size_t index = 1; index < gridPoints; ++index) {
      cumulativeIntegral += 0.5 * (weights[index - 1U] + weights[index]) * gridSpacing;
      cdf_[index] = cumulativeIntegral;
    }

    if (!isFinite(cumulativeIntegral) || cumulativeIntegral <= std::numeric_limits<double>::min()) {
      throw std::runtime_error("Maxwell-Juttner sampler integral must stay positive and finite.");
    }

    const double normalization = 1.0 / cumulativeIntegral;
    for (double &cdfValue : cdf_) {
      cdfValue *= normalization;
    }
    cdf_.back() = 1.0;
  }

  double MaxwellJuttnerMomentumSampler::sample(double unitUniform) const {
    if (unitUniform <= 0.0) {
      return momentumGrid_.front();
    }
    if (unitUniform >= 1.0) {
      return momentumGrid_.back();
    }

    const auto upper = std::lower_bound(cdf_.begin(), cdf_.end(), unitUniform);
    if (upper == cdf_.begin()) {
      return momentumGrid_.front();
    }
    if (upper == cdf_.end()) {
      return momentumGrid_.back();
    }

    const std::size_t upperIndex = static_cast<std::size_t>(std::distance(cdf_.begin(), upper));
    const std::size_t lowerIndex = upperIndex - 1U;
    const double lowerCdf = cdf_[lowerIndex];
    const double upperCdf = cdf_[upperIndex];
    if (upperCdf <= lowerCdf) {
      return momentumGrid_[upperIndex];
    }

    const double interpolationFraction = (unitUniform - lowerCdf) / (upperCdf - lowerCdf);
    return momentumGrid_[lowerIndex] + interpolationFraction * (momentumGrid_[upperIndex] - momentumGrid_[lowerIndex]);
  }

  double MaxwellJuttnerMomentumSampler::thermalWeight(double momentum, double mass, double temperature) {
    const double energy = std::sqrt(momentum * momentum + mass * mass);
    return momentum * momentum * std::exp(-energy / temperature);
  }

}  // namespace blastwave
