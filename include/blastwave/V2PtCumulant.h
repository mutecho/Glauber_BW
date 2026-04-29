#pragma once

#include <cstddef>
#include <vector>

namespace blastwave {

  struct V2PtTrack {
    double px = 0.0;
    double py = 0.0;
  };

  struct V2PtCumulantResult {
    std::vector<double> ptBinEdges;
    std::vector<double> v2Values;
    std::vector<double> v2Errors;
    double c2 = 0.0;
    std::size_t contributingEvents = 0;
  };

  struct V2PtCumulantEventContribution {
    double c2Numerator = 0.0;
    double c2Denominator = 0.0;
    std::vector<double> d2Numerators;
    std::vector<double> d2Denominators;
  };

  // Accumulate unit-weight two-particle cumulants and finalize differential
  // v2{2}(pT) with leave-one-out jackknife uncertainty.
  class V2PtCumulant {
   public:
    explicit V2PtCumulant(std::vector<double> ptBinEdges);

    void addEvent(const std::vector<V2PtTrack> &tracks);
    [[nodiscard]] V2PtCumulantResult finalize() const;

   private:
    [[nodiscard]] int findBin(double pt) const;

    std::vector<double> ptBinEdges_;
    std::vector<V2PtCumulantEventContribution> eventContributions_;
  };

}  // namespace blastwave
