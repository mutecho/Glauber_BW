#include "blastwave/DifferentialFlowCumulant.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace blastwave {

  namespace {

    struct RunningSums {
      double referenceNumerator = 0.0;
      double referenceDenominator = 0.0;
      std::vector<double> differentialNumerators;
      std::vector<double> differentialDenominators;
    };

    std::string observableName(int harmonic) {
      return "v" + std::to_string(harmonic) + "pt";
    }

    bool isFiniteNonNegative(double value) {
      return std::isfinite(value) && value >= 0.0;
    }

    void validateHarmonic(int harmonic) {
      if (harmonic != 2 && harmonic != 3) {
        throw std::invalid_argument("differential flow harmonic must be 2 or 3.");
      }
    }

    void validateEdges(const std::vector<double> &edges, int harmonic) {
      const std::string label = observableName(harmonic);
      if (edges.size() < 2U) {
        throw std::invalid_argument(label + " bin edges must contain at least two values.");
      }
      for (std::size_t iEdge = 0; iEdge < edges.size(); ++iEdge) {
        const double edge = edges[iEdge];
        if (!isFiniteNonNegative(edge)) {
          throw std::invalid_argument(label + " bin edge must be finite and non-negative at index " + std::to_string(iEdge) + ".");
        }
        if (iEdge > 0U && !(edges[iEdge] > edges[iEdge - 1U])) {
          throw std::invalid_argument(label + " bin edges must be strictly increasing.");
        }
      }
    }

    RunningSums buildTotals(const std::vector<DifferentialFlowCumulantEventContribution> &eventContributions, std::size_t nBins) {
      RunningSums totals;
      totals.differentialNumerators.assign(nBins, 0.0);
      totals.differentialDenominators.assign(nBins, 0.0);
      for (const DifferentialFlowCumulantEventContribution &event : eventContributions) {
        totals.referenceNumerator += event.referenceNumerator;
        totals.referenceDenominator += event.referenceDenominator;
        for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
          totals.differentialNumerators[iBin] += event.differentialNumerators[iBin];
          totals.differentialDenominators[iBin] += event.differentialDenominators[iBin];
        }
      }
      return totals;
    }

    std::vector<double> computeFlowFromSums(const RunningSums &sums, std::size_t nBins) {
      std::vector<double> values(nBins, 0.0);
      if (sums.referenceDenominator <= 0.0) {
        return values;
      }

      const double referenceCumulant = sums.referenceNumerator / sums.referenceDenominator;
      if (referenceCumulant <= 0.0 || !std::isfinite(referenceCumulant)) {
        return values;
      }

      const double inverseSqrtReference = 1.0 / std::sqrt(referenceCumulant);
      for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
        const double denominator = sums.differentialDenominators[iBin];
        if (denominator <= 0.0) {
          values[iBin] = 0.0;
          continue;
        }
        const double differentialCumulant = sums.differentialNumerators[iBin] / denominator;
        values[iBin] = differentialCumulant * inverseSqrtReference;
      }
      return values;
    }

  }  // namespace

  DifferentialFlowCumulant::DifferentialFlowCumulant(int harmonic, std::vector<double> ptBinEdges)
      : harmonic_(harmonic), ptBinEdges_(std::move(ptBinEdges)) {
    validateHarmonic(harmonic_);
    validateEdges(ptBinEdges_, harmonic_);
  }

  int DifferentialFlowCumulant::findBin(double pt) const {
    const double firstEdge = ptBinEdges_.front();
    const double lastEdge = ptBinEdges_.back();
    if (pt < firstEdge || pt > lastEdge) {
      return -1;
    }
    if (pt == lastEdge) {
      return static_cast<int>(ptBinEdges_.size() - 2U);
    }
    const auto upperEdge = std::upper_bound(ptBinEdges_.begin(), ptBinEdges_.end(), pt);
    if (upperEdge == ptBinEdges_.begin() || upperEdge == ptBinEdges_.end()) {
      return -1;
    }
    return static_cast<int>((upperEdge - ptBinEdges_.begin()) - 1);
  }

  void DifferentialFlowCumulant::addEvent(const std::vector<DifferentialFlowTrack> &tracks) {
    const int multiplicity = static_cast<int>(tracks.size());
    if (multiplicity < 2) {
      return;
    }

    const std::size_t nBins = ptBinEdges_.size() - 1U;
    std::vector<std::complex<double>> pVectorByBin(nBins, std::complex<double>(0.0, 0.0));
    std::vector<int> binCounts(nBins, 0);
    std::complex<double> qVector(0.0, 0.0);

    for (const DifferentialFlowTrack &track : tracks) {
      const double phi = std::atan2(track.py, track.px);
      const double harmonicPhi = static_cast<double>(harmonic_) * phi;
      const std::complex<double> eInPhi(std::cos(harmonicPhi), std::sin(harmonicPhi));
      qVector += eInPhi;

      const double pt = std::hypot(track.px, track.py);
      const int binIndex = findBin(pt);
      if (binIndex < 0) {
        continue;
      }
      pVectorByBin[static_cast<std::size_t>(binIndex)] += eInPhi;
      ++binCounts[static_cast<std::size_t>(binIndex)];
    }

    DifferentialFlowCumulantEventContribution eventContribution;
    eventContribution.referenceNumerator = std::norm(qVector) - static_cast<double>(multiplicity);
    eventContribution.referenceDenominator = static_cast<double>(multiplicity) * static_cast<double>(multiplicity - 1);
    eventContribution.differentialNumerators.assign(nBins, 0.0);
    eventContribution.differentialDenominators.assign(nBins, 0.0);

    for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
      const int mBin = binCounts[iBin];
      if (mBin <= 0) {
        continue;
      }
      const double differentialNumerator = std::real(pVectorByBin[iBin] * std::conj(qVector)) - static_cast<double>(mBin);
      const double differentialDenominator = static_cast<double>(mBin) * static_cast<double>(multiplicity - 1);
      eventContribution.differentialNumerators[iBin] = differentialNumerator;
      eventContribution.differentialDenominators[iBin] = differentialDenominator;
    }

    eventContributions_.push_back(std::move(eventContribution));
  }

  DifferentialFlowCumulantResult DifferentialFlowCumulant::finalize() const {
    const std::size_t nBins = ptBinEdges_.size() - 1U;
    const RunningSums totals = buildTotals(eventContributions_, nBins);
    const std::string label = observableName(harmonic_);
    if (totals.referenceDenominator <= 0.0) {
      throw std::runtime_error(label + " analysis failed: no events with multiplicity >= 2 contributed to c" + std::to_string(harmonic_) + "{2}.");
    }

    const double referenceCumulant = totals.referenceNumerator / totals.referenceDenominator;
    if (!(referenceCumulant > 0.0) || !std::isfinite(referenceCumulant)) {
      throw std::runtime_error(label + " analysis failed: global c" + std::to_string(harmonic_) + "{2} <= 0.");
    }

    DifferentialFlowCumulantResult result;
    result.harmonic = harmonic_;
    result.ptBinEdges = ptBinEdges_;
    result.values.assign(nBins, 0.0);
    result.errors.assign(nBins, 0.0);
    result.referenceCumulant = referenceCumulant;
    result.contributingEvents = eventContributions_.size();

    const double inverseSqrtReference = 1.0 / std::sqrt(referenceCumulant);
    for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
      if (totals.differentialDenominators[iBin] <= 0.0) {
        result.values[iBin] = 0.0;
        continue;
      }
      const double differentialCumulant = totals.differentialNumerators[iBin] / totals.differentialDenominators[iBin];
      result.values[iBin] = differentialCumulant * inverseSqrtReference;
    }

    const std::size_t nSamples = eventContributions_.size();
    if (nSamples < 2U) {
      return result;
    }

    std::vector<std::vector<double>> jackknifeValues(nBins, std::vector<double>(nSamples, 0.0));
    std::vector<double> jackknifeMeans(nBins, 0.0);
    for (std::size_t iSample = 0; iSample < nSamples; ++iSample) {
      RunningSums leaveOneOutSums;
      leaveOneOutSums.referenceNumerator = totals.referenceNumerator - eventContributions_[iSample].referenceNumerator;
      leaveOneOutSums.referenceDenominator = totals.referenceDenominator - eventContributions_[iSample].referenceDenominator;
      leaveOneOutSums.differentialNumerators.assign(nBins, 0.0);
      leaveOneOutSums.differentialDenominators.assign(nBins, 0.0);
      for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
        leaveOneOutSums.differentialNumerators[iBin] = totals.differentialNumerators[iBin] - eventContributions_[iSample].differentialNumerators[iBin];
        leaveOneOutSums.differentialDenominators[iBin] = totals.differentialDenominators[iBin] - eventContributions_[iSample].differentialDenominators[iBin];
      }

      const std::vector<double> leaveOneOutValues = computeFlowFromSums(leaveOneOutSums, nBins);
      for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
        jackknifeValues[iBin][iSample] = leaveOneOutValues[iBin];
        jackknifeMeans[iBin] += leaveOneOutValues[iBin];
      }
    }

    const double inverseSamples = 1.0 / static_cast<double>(nSamples);
    const double prefactor = static_cast<double>(nSamples - 1U) * inverseSamples;
    for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
      jackknifeMeans[iBin] *= inverseSamples;

      double varianceAccumulator = 0.0;
      for (double leaveOneOutValue : jackknifeValues[iBin]) {
        const double difference = leaveOneOutValue - jackknifeMeans[iBin];
        varianceAccumulator += difference * difference;
      }
      result.errors[iBin] = std::sqrt(prefactor * varianceAccumulator);
    }

    return result;
  }

}  // namespace blastwave
