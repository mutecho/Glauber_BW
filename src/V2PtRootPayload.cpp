#include "blastwave/io/V2PtRootPayload.h"

#include <TDirectory.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TObject.h>
#include <TVectorD.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "blastwave/io/RootOutputSchema.h"

namespace blastwave::io {

  namespace {

    void deleteAllCycles(TDirectory &directory, const char *objectName) {
      const std::string deletePattern = std::string(objectName) + ";*";
      directory.Delete(deletePattern.c_str());
    }

    TVectorD makeEdgesVector(const std::vector<double> &ptBinEdges) {
      TVectorD edges(static_cast<int>(ptBinEdges.size()));
      for (std::size_t iEdge = 0; iEdge < ptBinEdges.size(); ++iEdge) {
        edges[static_cast<int>(iEdge)] = ptBinEdges[iEdge];
      }
      return edges;
    }

    std::vector<double> toStdVector(const TVectorD &edges) {
      std::vector<double> values;
      values.reserve(static_cast<std::size_t>(edges.GetNoElements()));
      for (int iEdge = 0; iEdge < edges.GetNoElements(); ++iEdge) {
        values.push_back(edges[iEdge]);
      }
      return values;
    }

  }  // namespace

  void writeV2PtEdges(TDirectory &directory, const std::vector<double> &ptBinEdges) {
    directory.cd();
    deleteAllCycles(directory, kV2PtEdgesObjectName);
    TVectorD edges = makeEdgesVector(ptBinEdges);
    if (edges.Write(kV2PtEdgesObjectName, TObject::kOverwrite) <= 0) {
      throw std::runtime_error("Failed to write v2pt edge metadata object 'v2_2_pt_edges'.");
    }
  }

  std::vector<double> readV2PtEdgesOrThrow(TFile &inputFile) {
    auto *edgesObject = dynamic_cast<TVectorD *>(inputFile.Get(kV2PtEdgesObjectName));
    if (edgesObject == nullptr) {
      throw std::runtime_error("Missing ROOT metadata object 'v2_2_pt_edges'. Standalone v2pt analysis requires pT bin edges from file metadata.");
    }
    return toStdVector(*edgesObject);
  }

  void writeV2PtPayload(TDirectory &directory, const blastwave::V2PtCumulantResult &result) {
    const std::size_t nBins = result.v2Values.size();
    if (result.ptBinEdges.size() != nBins + 1U || result.v2Errors.size() != nBins) {
      throw std::invalid_argument("v2pt payload shape mismatch between edges/values/errors.");
    }

    directory.cd();
    writeV2PtEdges(directory, result.ptBinEdges);
    deleteAllCycles(directory, kV2PtHistogramName);
    deleteAllCycles(directory, kV2PtCanvasName);

    TH1D histogram(
        kV2PtHistogramName, "Differential v_{2}{2};p_{T} [GeV];v_{2}{2}", static_cast<int>(nBins), result.ptBinEdges.data());
    histogram.SetDirectory(nullptr);
    for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
      histogram.SetBinContent(static_cast<int>(iBin + 1U), result.v2Values[iBin]);
      histogram.SetBinError(static_cast<int>(iBin + 1U), result.v2Errors[iBin]);
    }

    TCanvas canvas(kV2PtCanvasName, "Differential v_{2}{2}", 820, 620);
    histogram.Draw("E1");
    canvas.Modified();
    canvas.Update();

    if (histogram.Write(kV2PtHistogramName, TObject::kOverwrite) <= 0) {
      throw std::runtime_error("Failed to write v2pt histogram object 'v2_2_pt'.");
    }
    if (canvas.Write(kV2PtCanvasName, TObject::kOverwrite) <= 0) {
      throw std::runtime_error("Failed to write v2pt canvas object 'v2_2_pt_canvas'.");
    }
  }

}  // namespace blastwave::io
