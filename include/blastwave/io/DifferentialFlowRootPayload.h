#pragma once

#include <optional>
#include <vector>

#include "blastwave/DifferentialFlowCumulant.h"

class TDirectory;
class TFile;

namespace blastwave::io {

  [[nodiscard]] const char *differentialFlowEdgesObjectName(int harmonic);
  [[nodiscard]] const char *differentialFlowHistogramName(int harmonic);
  [[nodiscard]] const char *differentialFlowCanvasName(int harmonic);

  // Persist pT-bin metadata independently from the histogram/canvas payload so
  // standalone analysis can discover enabled harmonics from the main output file.
  void writeDifferentialFlowEdges(TDirectory &directory, int harmonic, const std::vector<double> &ptBinEdges);
  [[nodiscard]] std::optional<std::vector<double>> readDifferentialFlowEdges(TFile &inputFile, int harmonic);
  [[nodiscard]] std::vector<double> readDifferentialFlowEdgesOrThrow(TFile &inputFile, int harmonic);

  // Write the full v_n{2}(pT) payload (edges + histogram + canvas) with
  // overwrite semantics on the harmonic-specific canonical ROOT object names.
  void writeDifferentialFlowPayload(TDirectory &directory, const blastwave::DifferentialFlowCumulantResult &result);

}  // namespace blastwave::io
