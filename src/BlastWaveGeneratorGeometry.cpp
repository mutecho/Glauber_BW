#include <algorithm>
#include <array>
#include <cmath>

#include "blastwave/BlastWaveGenerator.h"
#include "blastwave/FlowFieldModel.h"

namespace {

  constexpr double kPi = 3.14159265358979323846;
  constexpr double kTwoPi = 2.0 * kPi;
  constexpr double kTiny = 1.0e-12;

  std::array<int, 3> allocateRatioTotalCounts(int total, const std::array<double, 3> &weights) {
    std::array<int, 3> counts = {0, 0, 0};
    if (total <= 0) {
      return counts;
    }
    const double totalWeight = weights[0] + weights[1] + weights[2];
    const double invTotalWeight = 1.0 / totalWeight;
    std::array<double, 3> fractions = {weights[0] * total * invTotalWeight, weights[1] * total * invTotalWeight, weights[2] * total * invTotalWeight};
    int allocated = 0;
    for (std::size_t iComp = 0; iComp < fractions.size(); ++iComp) {
      counts[iComp] = static_cast<int>(std::floor(fractions[iComp]));
      allocated += counts[iComp];
    }

    int remaining = total - allocated;
    while (remaining > 0) {
      std::size_t bestIndex = 0;
      for (std::size_t iIndex = 1; iIndex < fractions.size(); ++iIndex) {
        if ((fractions[iIndex] - counts[iIndex]) > (fractions[bestIndex] - counts[bestIndex])) {
          bestIndex = iIndex;
        }
      }
      ++counts[bestIndex];
      --remaining;
    }
    return counts;
  }

  int roundNonNegativePoolCount(int backgroundCount, double componentWeight) {
    const double rawCount = static_cast<double>(backgroundCount) * std::max(0.0, componentWeight);
    return static_cast<int>(std::lround(rawCount));
  }

  std::array<int, 3> allocateIndependentPoolCounts(int backgroundCount, double rawA2, double rawA3) {
    const int ellipticCount = roundNonNegativePoolCount(backgroundCount, rawA2);
    int triangularCount = roundNonNegativePoolCount(backgroundCount, rawA3);
    if (rawA3 > kTiny && triangularCount < 3) {
      triangularCount = 3;
    }
    return {backgroundCount, ellipticCount, triangularCount};
  }

  void ensureMinimumComponentCount(std::array<int, 3> &componentCounts, std::size_t indexNeed, int minimumCount, int targetCount) {
    if (componentCounts[indexNeed] >= minimumCount || targetCount < minimumCount) {
      return;
    }

    while (componentCounts[indexNeed] < minimumCount) {
      int donor = -1;
      for (std::size_t iComponent = 0; iComponent < componentCounts.size(); ++iComponent) {
        if (iComponent == indexNeed || componentCounts[iComponent] <= 0) {
          continue;
        }
        if (donor < 0 || componentCounts[iComponent] > componentCounts[static_cast<std::size_t>(donor)]) {
          donor = static_cast<int>(iComponent);
        }
      }

      if (donor < 0) {
        break;
      }
      --componentCounts[static_cast<std::size_t>(donor)];
      ++componentCounts[indexNeed];
    }
  }

}  // namespace

namespace blastwave {

  // Build the participant list by sampling both nuclei and applying the
  // transverse collision criterion implied by the configured NN cross section.
  std::vector<BlastWaveGenerator::Nucleon> BlastWaveGenerator::sampleParticipants() {
    if (config_.initialGeometryMode == InitialGeometryMode::ResponseTest023) {
      const ResponseTest023EventGeometry eventGeometry = sampleResponseTest023EventGeometry();
      return sampleResponseTest023Participants(eventGeometry);
    }
    return sampleGlauberParticipants();
  }

  // Resolve one per-event set of response-test-023 template parameters. In non-fluctuating mode,
  // this falls back to the fixed config values so closure behavior stays unchanged.
  BlastWaveGenerator::ResponseTest023EventGeometry BlastWaveGenerator::sampleResponseTest023EventGeometry() {
    if (!config_.initialGeometryFluctuate) {
      return {config_.initialGeometrySourceCount,
              config_.initialGeometryA2,
              config_.initialGeometryA3,
              config_.initialGeometryR2x,
              config_.initialGeometryR2y,
              config_.initialGeometryR3,
              config_.initialGeometrySigma3};
    }

    const auto sampleInt = [this](int minValue, int maxValue) {
      return std::uniform_int_distribution<int>(minValue, maxValue)(rng_);
    };
    const auto sampleReal = [this](double minValue, double maxValue) {
      return std::uniform_real_distribution<double>(minValue, maxValue)(rng_);
    };

    return {sampleInt(config_.initialGeometrySourceCountMin, config_.initialGeometrySourceCountMax),
            sampleReal(config_.initialGeometryA2Min, config_.initialGeometryA2Max),
            sampleReal(config_.initialGeometryA3Min, config_.initialGeometryA3Max),
            sampleReal(config_.initialGeometryR2xMin, config_.initialGeometryR2xMax),
            sampleReal(config_.initialGeometryR2yMin, config_.initialGeometryR2yMax),
            sampleReal(config_.initialGeometryR3Min, config_.initialGeometryR3Max),
            sampleReal(config_.initialGeometrySigma3Min, config_.initialGeometrySigma3Max)};
  }

  // Default Glauber geometry keeps the existing sampled nuclei logic unchanged.
  std::vector<BlastWaveGenerator::Nucleon> BlastWaveGenerator::sampleGlauberParticipants() {
    std::vector<Nucleon> nucleusA;
    std::vector<Nucleon> nucleusB;
    nucleusA.reserve(static_cast<std::size_t>(config_.nucleonsPerNucleus));
    nucleusB.reserve(static_cast<std::size_t>(config_.nucleonsPerNucleus));

    const double halfImpactParameter = 0.5 * config_.impactParameter;
    for (int iNucleon = 0; iNucleon < config_.nucleonsPerNucleus; ++iNucleon) {
      nucleusA.push_back(sampleSingleNucleon(-halfImpactParameter, 0));
      nucleusB.push_back(sampleSingleNucleon(+halfImpactParameter, 1));
    }

    const double collisionDistanceSquared = config_.sigmaNN / kPi;
    for (Nucleon &nucleonA : nucleusA) {
      for (Nucleon &nucleonB : nucleusB) {
        const double dx = nucleonA.x - nucleonB.x;
        const double dy = nucleonA.y - nucleonB.y;
        const double transverseDistanceSquared = dx * dx + dy * dy;
        if (transverseDistanceSquared <= collisionDistanceSquared) {
          nucleonA.participant = true;
          nucleonB.participant = true;
        }
      }
    }

    std::vector<Nucleon> participants;
    participants.reserve(static_cast<std::size_t>(2 * config_.nucleonsPerNucleus));
    for (const Nucleon &nucleon : nucleusA) {
      if (nucleon.participant) {
        participants.push_back(nucleon);
      }
    }
    for (const Nucleon &nucleon : nucleusB) {
      if (nucleon.participant) {
        participants.push_back(nucleon);
      }
    }
    return participants;
  }

  // Build the response-test source cloud from three components with explicit 1:A2:A3
  // allocation or independent component pools, then recenter to remove
  // artificial dipoles in the synthetic template.
  std::vector<BlastWaveGenerator::Nucleon> BlastWaveGenerator::sampleResponseTest023Participants(
      const ResponseTest023EventGeometry &eventParameters) {
    // Use sampled per-event template parameters so one config can produce a stochastic
    // geometry template population while keeping the selected allocation contract explicit.
    const int sourceCount = std::max(1, eventParameters.sourceCount);
    const double rawA2 = std::max(0.0, eventParameters.a2);
    const double rawA3 = std::max(0.0, eventParameters.a3);
    const double rawA0 = 1.0;

    std::array<int, 3> componentCounts = {0, 0, 0};
    if (config_.initialGeometrySourceAllocationMode == InitialGeometrySourceAllocationMode::IndependentPools) {
      componentCounts = allocateIndependentPoolCounts(sourceCount, rawA2, rawA3);
    } else {
      const std::array<double, 3> weights = {rawA0, rawA2, rawA3};
      componentCounts = allocateRatioTotalCounts(sourceCount, weights);

      if (rawA2 > kTiny) {
        ensureMinimumComponentCount(componentCounts, 1, 1, sourceCount);
      }
      if (rawA3 > kTiny) {
        ensureMinimumComponentCount(componentCounts, 2, 3, sourceCount);
      }

      const int totalAssigned = componentCounts[0] + componentCounts[1] + componentCounts[2];
      if (totalAssigned > sourceCount && componentCounts[0] > 0) {
        componentCounts[0] -= std::min(componentCounts[0], totalAssigned - sourceCount);
      }
      if (componentCounts[0] + componentCounts[1] + componentCounts[2] < sourceCount && componentCounts[0] >= 0) {
        componentCounts[0] += sourceCount - (componentCounts[0] + componentCounts[1] + componentCounts[2]);
      }
    }

    const int totalSourceCount = componentCounts[0] + componentCounts[1] + componentCounts[2];
    const double cosPhi2 = std::cos(config_.initialGeometryPhi2);
    const double sinPhi2 = std::sin(config_.initialGeometryPhi2);
    std::normal_distribution<double> backgroundDistX(0.0, config_.initialGeometryR0);
    std::normal_distribution<double> ellipseDistX(0.0, eventParameters.r2x);
    std::normal_distribution<double> ellipseDistY(0.0, eventParameters.r2y);
    std::normal_distribution<double> triangleDist(0.0, eventParameters.sigma3);
    std::vector<WeightedTransversePoint> sourcePoints;
    sourcePoints.reserve(static_cast<std::size_t>(totalSourceCount));

    for (int i = 0; i < componentCounts[0]; ++i) {
      sourcePoints.push_back({backgroundDistX(rng_), backgroundDistX(rng_), 1.0});
    }
    for (int i = 0; i < componentCounts[1]; ++i) {
      const double dx = ellipseDistX(rng_);
      const double dy = ellipseDistY(rng_);
      sourcePoints.push_back({dx * cosPhi2 - dy * sinPhi2, dx * sinPhi2 + dy * cosPhi2, 1.0});
    }

    const int nForEachPeak = componentCounts[2] / 3;
    const int tailPeaks = componentCounts[2] % 3;
    const double r3 = std::max(0.0, eventParameters.r3);
    for (int peak = 0; peak < 3; ++peak) {
      const int nForPeak = nForEachPeak + (peak < tailPeaks ? 1 : 0);
      const double peakPhi = config_.initialGeometryPhi3 + kTwoPi * static_cast<double>(peak) / 3.0;
      const double peakX = r3 * std::cos(peakPhi);
      const double peakY = r3 * std::sin(peakPhi);
      for (int i = 0; i < nForPeak; ++i) {
        sourcePoints.push_back({peakX + triangleDist(rng_), peakY + triangleDist(rng_), 1.0});
      }
    }

    if (static_cast<int>(sourcePoints.size()) != totalSourceCount) {
      sourcePoints.resize(static_cast<std::size_t>(totalSourceCount), sourcePoints.empty() ? WeightedTransversePoint{0.0, 0.0, 1.0} : sourcePoints.back());
    }

    double centroidX = 0.0;
    double centroidY = 0.0;
    for (const WeightedTransversePoint &point : sourcePoints) {
      centroidX += point.x;
      centroidY += point.y;
    }
    centroidX /= static_cast<double>(sourcePoints.size());
    centroidY /= static_cast<double>(sourcePoints.size());

    for (WeightedTransversePoint &point : sourcePoints) {
      point.x -= centroidX;
      point.y -= centroidY;
    }

    std::vector<Nucleon> participants;
    participants.reserve(sourcePoints.size());
    for (const WeightedTransversePoint &point : sourcePoints) {
      participants.push_back({point.x, point.y, 0.0, -1});
    }
    return participants;
  }

  // Sample one Woods-Saxon nucleon in the nucleus rest frame and then shift the
  // x coordinate so the two nuclei realize the requested impact parameter.
  BlastWaveGenerator::Nucleon BlastWaveGenerator::sampleSingleNucleon(double xShift, int nucleusId) {
    const double radialMax = config_.woodsSaxonRadius + 10.0 * config_.woodsSaxonDiffuseness;
    std::uniform_real_distribution<double> cosThetaDistribution(-1.0, 1.0);
    std::uniform_real_distribution<double> phiDistribution(0.0, kTwoPi);

    while (true) {
      const double radius = radialMax * unitUniform_(rng_);
      const double density = 1.0 / (1.0 + std::exp((radius - config_.woodsSaxonRadius) / config_.woodsSaxonDiffuseness));
      const double acceptance = density * (radius * radius) / (radialMax * radialMax);
      if (unitUniform_(rng_) > acceptance) {
        continue;
      }

      const double cosTheta = cosThetaDistribution(rng_);
      const double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
      const double phi = phiDistribution(rng_);

      Nucleon nucleon;
      nucleon.x = radius * sinTheta * std::cos(phi) + xShift;
      nucleon.y = radius * sinTheta * std::sin(phi);
      nucleon.z = radius * cosTheta;
      nucleon.nucleusId = nucleusId;
      return nucleon;
    }
  }

  // Convert generator-owned participants into the shared event-medium state so
  // geometry, density reconstruction, and future density evolution stay out of
  // the generator orchestration layer.
  EventMedium BlastWaveGenerator::buildMedium(const std::vector<Nucleon> &participants) const {
    std::vector<WeightedTransversePoint> weightedPoints;
    weightedPoints.reserve(participants.size());
    for (const Nucleon &participant : participants) {
      weightedPoints.push_back({participant.x, participant.y, 1.0});
    }

    return buildEventMedium(weightedPoints,
                            {config_.densityEvolutionMode,
                             config_.flowDensitySigma,
                             config_.affineLambdaIn,
                             config_.affineLambdaOut,
                             config_.affineSigmaEvo,
                             config_.gradientSigmaEm,
                             config_.gradientSigmaDyn,
                             config_.gradientDensityFloorFraction,
                             config_.gradientDensityCutoffFraction});
  }

}  // namespace blastwave
