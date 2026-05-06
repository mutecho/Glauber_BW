#pragma once

#include <string>
#include <vector>

#include "blastwave/BlastWaveGenerator.h"

namespace blastwave::app {

  enum class ProgressMode { Auto, Enabled, Disabled };
  enum class FlowPtOutputMode { SameFile, SeparateFile };

  struct RunOptions {
    blastwave::BlastWaveConfig config;
    std::string outputPath = "blastwave.root";
    ProgressMode progressMode = ProgressMode::Auto;
    std::vector<double> v2PtBinEdges;
    std::vector<double> v3PtBinEdges;
    FlowPtOutputMode flowPtOutputMode = FlowPtOutputMode::SameFile;
    std::string flowPtOutputPath;
    bool hasFlowPtOutputModeOption = false;
    bool hasFlowPtOutputPathOption = false;
  };

  // Keep progress reporting in the CLI layer so the generator core stays
  // side-effect free and deterministic under tests.
  class ProgressReporter {
   public:
    ProgressReporter(int totalEvents, ProgressMode progressMode);
    ~ProgressReporter();

    void update(int completedEvents);
    void finish();

   private:
    static constexpr int kBarWidth = 20;

    int totalEvents_ = 0;
    int lastPercent_ = -1;
    bool enabled_ = false;
    bool drawn_ = false;
    bool lineClosed_ = false;
  };

  void printGenerateUsage(const char *programName);
  [[nodiscard]] RunOptions parseRunOptions(int argc, char **argv, bool &showHelp);

}  // namespace blastwave::app
