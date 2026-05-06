#include "blastwave/io/DifferentialFlowRootPayload.h"

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

    void validateHarmonic(int harmonic) {
      if (harmonic != 2 && harmonic != 3) {
        throw std::invalid_argument("differential flow ROOT payload harmonic must be 2 or 3.");
      }
    }

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

    std::string flowTitle(int harmonic) {
      return "Differential v_{" + std::to_string(harmonic) + "}{2}";
    }

  }  // namespace

  const char *differentialFlowEdgesObjectName(int harmonic) {
    validateHarmonic(harmonic);
    return harmonic == 2 ? kV2PtEdgesObjectName : kV3PtEdgesObjectName;
  }

  const char *differentialFlowHistogramName(int harmonic) {
    validateHarmonic(harmonic);
    return harmonic == 2 ? kV2PtHistogramName : kV3PtHistogramName;
  }

  const char *differentialFlowCanvasName(int harmonic) {
    validateHarmonic(harmonic);
    return harmonic == 2 ? kV2PtCanvasName : kV3PtCanvasName;
  }

  void writeDifferentialFlowEdges(TDirectory &directory, int harmonic, const std::vector<double> &ptBinEdges) {
    const char *edgesObjectName = differentialFlowEdgesObjectName(harmonic);
    directory.cd();
    deleteAllCycles(directory, edgesObjectName);
    TVectorD edges = makeEdgesVector(ptBinEdges);
    if (edges.Write(edgesObjectName, TObject::kOverwrite) <= 0) {
      throw std::runtime_error("Failed to write differential-flow edge metadata object '" + std::string(edgesObjectName) + "'.");
    }
  }

  std::optional<std::vector<double>> readDifferentialFlowEdges(TFile &inputFile, int harmonic) {
    const char *edgesObjectName = differentialFlowEdgesObjectName(harmonic);
    auto *rawObject = inputFile.Get(edgesObjectName);
    if (rawObject == nullptr) {
      return std::nullopt;
    }
    auto *edgesObject = dynamic_cast<TVectorD *>(rawObject);
    if (edgesObject == nullptr) {
      throw std::runtime_error("Input object '" + std::string(edgesObjectName) + "' is not a TVectorD.");
    }
    return toStdVector(*edgesObject);
  }

  std::vector<double> readDifferentialFlowEdgesOrThrow(TFile &inputFile, int harmonic) {
    const std::optional<std::vector<double>> edges = readDifferentialFlowEdges(inputFile, harmonic);
    if (!edges.has_value()) {
      const char *edgesObjectName = differentialFlowEdgesObjectName(harmonic);
      throw std::runtime_error("Missing ROOT metadata object '" + std::string(edgesObjectName) + "'. Standalone flowpt analysis requires pT bin edges from file metadata.");
    }
    return *edges;
  }

  void writeDifferentialFlowPayload(TDirectory &directory, const blastwave::DifferentialFlowCumulantResult &result) {
    const std::size_t nBins = result.values.size();
    if (result.ptBinEdges.size() != nBins + 1U || result.errors.size() != nBins) {
      throw std::invalid_argument("differential-flow payload shape mismatch between edges/values/errors.");
    }

    const char *histogramName = differentialFlowHistogramName(result.harmonic);
    const char *canvasName = differentialFlowCanvasName(result.harmonic);
    directory.cd();
    writeDifferentialFlowEdges(directory, result.harmonic, result.ptBinEdges);
    deleteAllCycles(directory, histogramName);
    deleteAllCycles(directory, canvasName);

    const std::string title = flowTitle(result.harmonic);
    const std::string histogramTitle = title + ";p_{T} [GeV];v_{" + std::to_string(result.harmonic) + "}{2}";
    TH1D histogram(histogramName, histogramTitle.c_str(), static_cast<int>(nBins), result.ptBinEdges.data());
    histogram.SetDirectory(nullptr);
    for (std::size_t iBin = 0; iBin < nBins; ++iBin) {
      histogram.SetBinContent(static_cast<int>(iBin + 1U), result.values[iBin]);
      histogram.SetBinError(static_cast<int>(iBin + 1U), result.errors[iBin]);
    }

    TCanvas canvas(canvasName, title.c_str(), 820, 620);
    histogram.Draw("E1");
    canvas.Modified();
    canvas.Update();

    if (histogram.Write(histogramName, TObject::kOverwrite) <= 0) {
      throw std::runtime_error("Failed to write differential-flow histogram object '" + std::string(histogramName) + "'.");
    }
    if (canvas.Write(canvasName, TObject::kOverwrite) <= 0) {
      throw std::runtime_error("Failed to write differential-flow canvas object '" + std::string(canvasName) + "'.");
    }
  }

}  // namespace blastwave::io
