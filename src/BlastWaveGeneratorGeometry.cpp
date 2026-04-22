#include <algorithm>
#include <cmath>

#include "blastwave/BlastWaveGenerator.h"
#include "blastwave/FlowFieldModel.h"

namespace {

  constexpr double kPi = 3.14159265358979323846;
  constexpr double kTwoPi = 2.0 * kPi;

}  // namespace

namespace blastwave {

  // Build the participant list by sampling both nuclei and applying the
  // transverse collision criterion implied by the configured NN cross section.
  std::vector<BlastWaveGenerator::Nucleon> BlastWaveGenerator::sampleParticipants() {
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

  // Convert the generator-owned participant records into the shared ROOT-free
  // covariance-ellipse module so geometry math stays implemented in one place.
  FlowEllipseInfo BlastWaveGenerator::computeParticipantShape(const std::vector<Nucleon> &participants) const {
    std::vector<WeightedTransversePoint> weightedPoints;
    weightedPoints.reserve(participants.size());
    for (const Nucleon &participant : participants) {
      weightedPoints.push_back({participant.x, participant.y, 1.0});
    }

    return computeFlowEllipseInfo(weightedPoints);
  }

}  // namespace blastwave
