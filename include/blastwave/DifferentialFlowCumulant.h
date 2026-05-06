#pragma once

#include <cstddef>
#include <vector>

namespace blastwave {

  struct DifferentialFlowTrack {
    double px = 0.0;
    double py = 0.0;
  };

  struct DifferentialFlowCumulantResult {
    int harmonic = 0;
    std::vector<double> ptBinEdges;
    std::vector<double> values;
    std::vector<double> errors;
    double referenceCumulant = 0.0;
    std::size_t contributingEvents = 0;
  };

  struct DifferentialFlowCumulantEventContribution {
    double referenceNumerator = 0.0;
    double referenceDenominator = 0.0;
    std::vector<double> differentialNumerators;
    std::vector<double> differentialDenominators;
  };

  // Accumulate unit-weight two-particle cumulants for harmonic n=2 or n=3 and
  // finalize v_n{2}(pT) with leave-one-event-out jackknife uncertainties.
  class DifferentialFlowCumulant {
   public:
    DifferentialFlowCumulant(int harmonic, std::vector<double> ptBinEdges);

    void addEvent(const std::vector<DifferentialFlowTrack> &tracks);
    [[nodiscard]] DifferentialFlowCumulantResult finalize() const;

   private:
    [[nodiscard]] int findBin(double pt) const;

    int harmonic_ = 0;
    std::vector<double> ptBinEdges_;
    std::vector<DifferentialFlowCumulantEventContribution> eventContributions_;
  };

}  // namespace blastwave
