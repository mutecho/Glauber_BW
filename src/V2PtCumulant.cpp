#include "blastwave/V2PtCumulant.h"

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
      double c2Numerator = 0.0;
      double c2Denominator = 0.0;
      std::vector<double> d2Numerators;
      std::vector<double> d2Denominators;
    };

    bool isFiniteNonNegative(double value) {
      return std::isfinite(value) && value >= 0.0;
    }

    void validateEdges(const std::vector<double> &edges) {
      if (edges.size() < 2U) {
        throw std::invalid_argument("v2pt bin edges must contain at least two values.");
      }
      for (std::size_t iEdge = 0; iEdge < edges.size(); ++iEdge) {
        const double edge = edges[iEdge];
        if (!isFiniteNonNegative(edge)) {
          throw std::invalid_argument("v2pt bin edge must be finite and non-negative at index " + std::to_string(iEdge) + ".");
        }
        if (iEdge > 0U && !(edges[iEdge] > edges[iEdge - 1U])) {
          throw std::invalid_argument("v2pt bin edges must be strictly increasing.");
        }
      }
    }

    RunningSums buildTotals(const std::vector<V2PtCumulantEventContribution> &eventContributions, std::size_t nBins) {
      RunningSums totals;
      totals.d2Numerators.assign(nBins, 0.0);
      totals.d2Denominators.assign(nBins, 0.0);
      for (const V2PtCumulantEventContribution &event : eventContributions) {
        totals.c2Numerator += event.c2Numerator;
        totals.c2Denominator += event.c2Denominator;
        for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
          totals.d2Numerators[iBin] += event.d2Numerators[iBin];
          totals.d2Denominators[iBin] += event.d2Denominators[iBin];
        }
      }
      return totals;
    }

    std::vector<double> computeV2FromSums(const RunningSums &sums, std::size_t nBins) {
      std::vector<double> v2Values(nBins, 0.0);
      if (sums.c2Denominator <= 0.0) {
        return v2Values;
      }

      const double c2 = sums.c2Numerator / sums.c2Denominator;
      if (c2 <= 0.0 || !std::isfinite(c2)) {
        return v2Values;
      }

      const double inverseSqrtC2 = 1.0 / std::sqrt(c2);
      for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
        const double denominator = sums.d2Denominators[iBin];
        if (denominator <= 0.0) {
          v2Values[iBin] = 0.0;
          continue;
        }
        const double d2 = sums.d2Numerators[iBin] / denominator;
        v2Values[iBin] = d2 * inverseSqrtC2;
      }
      return v2Values;
    }

  }  // namespace

  V2PtCumulant::V2PtCumulant(std::vector<double> ptBinEdges) : ptBinEdges_(std::move(ptBinEdges)) {
    validateEdges(ptBinEdges_);
  }

  int V2PtCumulant::findBin(double pt) const {
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

  void V2PtCumulant::addEvent(const std::vector<V2PtTrack> &tracks) {
    const int multiplicity = static_cast<int>(tracks.size());
    if (multiplicity < 2) {
      return;
    }

    const std::size_t nBins = ptBinEdges_.size() - 1U;
    std::vector<std::complex<double>> p2(nBins, std::complex<double>(0.0, 0.0));
    std::vector<int> binCounts(nBins, 0);
    std::complex<double> q2(0.0, 0.0);

    for (const V2PtTrack &track : tracks) {
      const double phi = std::atan2(track.py, track.px);
      const std::complex<double> e2iPhi(std::cos(2.0 * phi), std::sin(2.0 * phi));
      q2 += e2iPhi;

      const double pt = std::hypot(track.px, track.py);
      const int binIndex = findBin(pt);
      if (binIndex < 0) {
        continue;
      }
      p2[static_cast<std::size_t>(binIndex)] += e2iPhi;
      ++binCounts[static_cast<std::size_t>(binIndex)];
    }

    V2PtCumulantEventContribution eventContribution;
    eventContribution.c2Numerator = std::norm(q2) - static_cast<double>(multiplicity);
    eventContribution.c2Denominator = static_cast<double>(multiplicity) * static_cast<double>(multiplicity - 1);
    eventContribution.d2Numerators.assign(nBins, 0.0);
    eventContribution.d2Denominators.assign(nBins, 0.0);

    for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
      const int mBin = binCounts[iBin];
      if (mBin <= 0) {
        continue;
      }
      const double differentialNumerator = std::real(p2[iBin] * std::conj(q2)) - static_cast<double>(mBin);
      const double differentialDenominator = static_cast<double>(mBin) * static_cast<double>(multiplicity - 1);
      eventContribution.d2Numerators[iBin] = differentialNumerator;
      eventContribution.d2Denominators[iBin] = differentialDenominator;
    }

    eventContributions_.push_back(std::move(eventContribution));
  }

  V2PtCumulantResult V2PtCumulant::finalize() const {
    const std::size_t nBins = ptBinEdges_.size() - 1U;
    const RunningSums totals = buildTotals(eventContributions_, nBins);
    if (totals.c2Denominator <= 0.0) {
      throw std::runtime_error("v2pt analysis failed: no events with multiplicity >= 2 contributed to c2{2}.");
    }

    const double c2 = totals.c2Numerator / totals.c2Denominator;
    if (!(c2 > 0.0) || !std::isfinite(c2)) {
      throw std::runtime_error("v2pt analysis failed: global c2{2} <= 0.");
    }

    V2PtCumulantResult result;
    result.ptBinEdges = ptBinEdges_;
    result.v2Values.assign(nBins, 0.0);
    result.v2Errors.assign(nBins, 0.0);
    result.c2 = c2;
    result.contributingEvents = eventContributions_.size();

    const double inverseSqrtC2 = 1.0 / std::sqrt(c2);
    for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
      if (totals.d2Denominators[iBin] <= 0.0) {
        result.v2Values[iBin] = 0.0;
        continue;
      }
      const double d2 = totals.d2Numerators[iBin] / totals.d2Denominators[iBin];
      result.v2Values[iBin] = d2 * inverseSqrtC2;
    }

    const std::size_t nSamples = eventContributions_.size();
    if (nSamples < 2U) {
      return result;
    }

    std::vector<std::vector<double>> jackknifeValues(nBins, std::vector<double>(nSamples, 0.0));
    std::vector<double> jackknifeMeans(nBins, 0.0);
    for (std::size_t iSample = 0; iSample < nSamples; ++iSample) {
      RunningSums leaveOneOutSums;
      leaveOneOutSums.c2Numerator = totals.c2Numerator - eventContributions_[iSample].c2Numerator;
      leaveOneOutSums.c2Denominator = totals.c2Denominator - eventContributions_[iSample].c2Denominator;
      leaveOneOutSums.d2Numerators.assign(nBins, 0.0);
      leaveOneOutSums.d2Denominators.assign(nBins, 0.0);
      for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
        leaveOneOutSums.d2Numerators[iBin] = totals.d2Numerators[iBin] - eventContributions_[iSample].d2Numerators[iBin];
        leaveOneOutSums.d2Denominators[iBin] = totals.d2Denominators[iBin] - eventContributions_[iSample].d2Denominators[iBin];
      }

      const std::vector<double> leaveOneOutV2 = computeV2FromSums(leaveOneOutSums, nBins);
      for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
        jackknifeValues[iBin][iSample] = leaveOneOutV2[iBin];
        jackknifeMeans[iBin] += leaveOneOutV2[iBin];
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
      result.v2Errors[iBin] = std::sqrt(prefactor * varianceAccumulator);
    }

    return result;
  }

}  // namespace blastwave
