#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "blastwave/BlastWaveGenerator.h"
#include "blastwave/MaxwellJuttnerMomentumSampler.h"

namespace {

  void require(bool condition, const std::string &message) {
    if (!condition) {
      throw std::runtime_error(message);
    }
  }

  double thermalWeight(double momentumMagnitude, double mass, double temperature) {
    const double energy = std::sqrt(momentumMagnitude * momentumMagnitude + mass * mass);
    return momentumMagnitude * momentumMagnitude * std::exp(-energy / temperature);
  }

  double integrateWeight(double mass, double temperature, double lower, double upper, int steps = 64) {
    if (upper <= lower) {
      return 0.0;
    }

    const double stepWidth = (upper - lower) / static_cast<double>(steps);
    double integral = 0.0;
    for (int step = 0; step < steps; ++step) {
      const double momentumMagnitude = lower + (static_cast<double>(step) + 0.5) * stepWidth;
      integral += thermalWeight(momentumMagnitude, mass, temperature);
    }
    return integral * stepWidth;
  }

  std::vector<double> buildExpectedHistogram(double mass, double temperature, double pMax, std::size_t bins) {
    std::vector<double> histogram(bins, 0.0);
    const double binWidth = pMax / static_cast<double>(bins);
    for (std::size_t index = 0; index < bins; ++index) {
      const double lowerEdge = static_cast<double>(index) * binWidth;
      histogram[index] = integrateWeight(mass, temperature, lowerEdge, lowerEdge + binWidth);
    }

    const double normalization = std::accumulate(histogram.begin(), histogram.end(), 0.0);
    require(normalization > 0.0, "Expected histogram normalization must stay positive.");
    for (double &value : histogram) {
      value /= normalization;
    }
    return histogram;
  }

  void verifySamplerTable(const blastwave::MaxwellJuttnerMomentumSampler &sampler) {
    const std::vector<double> &grid = sampler.momentumGrid();
    const std::vector<double> &cdf = sampler.cdf();

    require(grid.size() == cdf.size(), "Momentum grid and CDF sizes must match.");
    require(grid.size() >= 2U, "Momentum grid must contain at least two points.");
    require(std::abs(grid.front()) <= 1.0e-12, "Momentum grid must start at zero.");
    require(std::abs(grid.back() - sampler.pMax()) <= 1.0e-12, "Momentum grid must end at pMax.");
    require(std::abs(cdf.front()) <= 1.0e-12, "CDF must start at zero.");
    require(std::abs(cdf.back() - 1.0) <= 1.0e-12, "CDF must end at one.");

    for (std::size_t index = 0; index < grid.size(); ++index) {
      require(std::isfinite(grid[index]), "Momentum grid contains NaN or Inf.");
      require(std::isfinite(cdf[index]), "CDF contains NaN or Inf.");
      if (index > 0U) {
        require(grid[index] >= grid[index - 1U], "Momentum grid must be monotonic.");
        require(cdf[index] >= cdf[index - 1U], "CDF must be monotonic.");
      }
    }
  }

  void runShapeTest(double mass, double temperature, double pMax, std::size_t gridPoints, std::uint64_t seed) {
    blastwave::MaxwellJuttnerMomentumSampler sampler(mass, temperature, pMax, gridPoints);
    verifySamplerTable(sampler);

    constexpr std::size_t kBins = 80U;
    constexpr std::size_t kSamples = 250000U;
    const double binWidth = pMax / static_cast<double>(kBins);
    std::vector<std::size_t> counts(kBins, 0U);

    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> unitUniform(0.0, 1.0);
    for (std::size_t sampleIndex = 0; sampleIndex < kSamples; ++sampleIndex) {
      const double momentumMagnitude = sampler.sample(unitUniform(rng));
      require(std::isfinite(momentumMagnitude), "Sampled momentum magnitude must be finite.");
      require(momentumMagnitude >= 0.0, "Sampled momentum magnitude must be non-negative.");
      require(momentumMagnitude <= pMax, "Sampled momentum magnitude exceeded pMax.");

      const std::size_t bin = std::min(static_cast<std::size_t>(momentumMagnitude / binWidth), kBins - 1U);
      ++counts[bin];
    }

    const std::vector<double> expected = buildExpectedHistogram(mass, temperature, pMax, kBins);

    double chiSquare = 0.0;
    int populatedBins = 0;
    for (std::size_t index = 0; index < counts.size(); ++index) {
      const double expectedCount = expected[index] * static_cast<double>(kSamples);
      if (expectedCount < 20.0) {
        continue;
      }

      const double difference = static_cast<double>(counts[index]) - expectedCount;
      chiSquare += difference * difference / expectedCount;
      ++populatedBins;
    }

    require(populatedBins > 10, "Shape test needs enough populated bins.");
    require(chiSquare / static_cast<double>(populatedBins) < 3.0, "Histogram shape deviates too much from the theoretical Maxwell-Juttner spectrum.");

    const double normalization = integrateWeight(mass, temperature, 0.0, pMax, 2048);
    require(normalization > 0.0, "Spectrum normalization must stay positive.");
    const double lowEdge = 0.4;
    const double highEdge = 1.5;

    std::size_t lowCount = 0U;
    std::size_t highCount = 0U;
    for (std::size_t index = 0; index < counts.size(); ++index) {
      const double binCenter = (static_cast<double>(index) + 0.5) * binWidth;
      if (binCenter < lowEdge) {
        lowCount += counts[index];
      }
      if (binCenter >= highEdge) {
        highCount += counts[index];
      }
    }

    const double expectedLowFraction = integrateWeight(mass, temperature, 0.0, lowEdge, 512) / normalization;
    const double expectedHighFraction = integrateWeight(mass, temperature, highEdge, pMax, 512) / normalization;
    const double observedLowFraction = static_cast<double>(lowCount) / static_cast<double>(kSamples);
    const double observedHighFraction = static_cast<double>(highCount) / static_cast<double>(kSamples);
    const double lowSigma = std::sqrt(expectedLowFraction * (1.0 - expectedLowFraction) / static_cast<double>(kSamples));
    const double highSigma = std::sqrt(expectedHighFraction * (1.0 - expectedHighFraction) / static_cast<double>(kSamples));

    require(std::abs(observedLowFraction - expectedLowFraction) <= 6.0 * lowSigma + 0.002, "Low-momentum fraction shows a systematic bias.");
    require(std::abs(observedHighFraction - expectedHighFraction) <= 6.0 * highSigma + 0.002, "High-momentum tail shows a systematic bias.");
  }

  void runStabilityRegression(double mass, double temperature) {
    blastwave::MaxwellJuttnerMomentumSampler baseline(mass, temperature, 8.0, 4096U);
    blastwave::MaxwellJuttnerMomentumSampler refined(mass, temperature, 10.0, 8192U);

    constexpr std::size_t kSamples = 200000U;
    std::mt19937_64 rng(987654321ULL);
    std::uniform_real_distribution<double> unitUniform(0.0, 1.0);

    double baselineMeanMomentum = 0.0;
    double refinedMeanMomentum = 0.0;
    double baselineMeanEnergy = 0.0;
    double refinedMeanEnergy = 0.0;
    for (std::size_t sampleIndex = 0; sampleIndex < kSamples; ++sampleIndex) {
      const double unitSample = unitUniform(rng);
      const double baselineMomentum = baseline.sample(unitSample);
      const double refinedMomentum = refined.sample(unitSample);

      baselineMeanMomentum += baselineMomentum;
      refinedMeanMomentum += refinedMomentum;
      baselineMeanEnergy += std::sqrt(baselineMomentum * baselineMomentum + mass * mass);
      refinedMeanEnergy += std::sqrt(refinedMomentum * refinedMomentum + mass * mass);
    }

    const double inverseCount = 1.0 / static_cast<double>(kSamples);
    baselineMeanMomentum *= inverseCount;
    refinedMeanMomentum *= inverseCount;
    baselineMeanEnergy *= inverseCount;
    refinedMeanEnergy *= inverseCount;

    require(std::abs(baselineMeanMomentum - refinedMeanMomentum) < 2.0e-3, "Mean momentum is too sensitive to MJ table refinement.");
    require(std::abs(baselineMeanEnergy - refinedMeanEnergy) < 2.0e-3, "Mean energy is too sensitive to MJ table refinement.");
  }

  void runZeroTemperatureGeneratorTest(blastwave::ThermalSamplerMode mode) {
    blastwave::BlastWaveConfig config;
    config.seed = 424242;
    config.nucleonsPerNucleus = 4;
    config.impactParameter = 0.0;
    config.temperature = 0.0;
    config.thermalSamplerMode = mode;
    config.rho0 = 0.0;
    config.kappa2 = 0.0;
    config.sigmaEta = 0.0;
    config.etaPlateauHalfWidth = 0.0;
    config.smearSigma = 0.0;
    config.sigmaNN = 1.0e6;
    config.nbdMu = 50.0;
    config.nbdK = 2.0;

    blastwave::BlastWaveGenerator generator(config);
    const blastwave::GeneratedEvent event = generator.generateEvent(0);
    require(!event.particles.empty(), "Zero-temperature generator test needs at least one particle.");

    for (const blastwave::ParticleRecord &particle : event.particles) {
      require(std::abs(particle.px) <= 1.0e-12, "Zero-temperature px must vanish.");
      require(std::abs(particle.py) <= 1.0e-12, "Zero-temperature py must vanish.");
      require(std::abs(particle.pz) <= 1.0e-12, "Zero-temperature pz must vanish.");
      require(std::abs(particle.energy - particle.mass) <= 1.0e-12, "Zero-temperature particles must satisfy E = m.");
    }
  }

}  // namespace

int main() {
  try {
    runShapeTest(0.13957, 0.2, 8.0, 4096U, 12345ULL);
    runShapeTest(0.938272, 0.12, 8.0, 4096U, 67890ULL);
    runStabilityRegression(0.13957, 0.2);
    runZeroTemperatureGeneratorTest(blastwave::ThermalSamplerMode::MaxwellJuttner);
    runZeroTemperatureGeneratorTest(blastwave::ThermalSamplerMode::Gamma);

    std::cout << "Maxwell-Juttner core tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Maxwell-Juttner core tests failed: " << error.what() << '\n';
    return 1;
  }
}
