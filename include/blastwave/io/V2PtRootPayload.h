#pragma once

#include <vector>

#include "blastwave/V2PtCumulant.h"

class TDirectory;
class TFile;

namespace blastwave::io {

  // Persist pT-bin metadata independently from the analysis payload so the
  // standalone analyzer can always discover binning from the main output file.
  void writeV2PtEdges(TDirectory &directory, const std::vector<double> &ptBinEdges);
  [[nodiscard]] std::vector<double> readV2PtEdgesOrThrow(TFile &inputFile);

  // Write the full v2{2}(pT) payload (edges + histogram + canvas) with
  // overwrite semantics on the canonical object names.
  void writeV2PtPayload(TDirectory &directory, const blastwave::V2PtCumulantResult &result);

}  // namespace blastwave::io
