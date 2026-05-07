#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <iosfwd>
#include <mutex>
#include <string>
#include <thread>
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

  // Progress snapshots are pure render inputs, which keeps ETA/spinner
  // formatting testable without starting a terminal heartbeat thread.
  struct ProgressRenderSnapshot {
    int totalEvents = 0;
    int completedEvents = 0;
    int activityFrame = 0;
    std::chrono::steady_clock::duration elapsed{};
  };

  [[nodiscard]] std::string formatProgressLine(const ProgressRenderSnapshot &snapshot);

  // Keep progress reporting in the CLI layer so the generator core stays
  // side-effect free and deterministic under tests.
  class ProgressReporter {
   public:
    ProgressReporter(int totalEvents, ProgressMode progressMode);
    ~ProgressReporter();

    void update(int completedEvents);
    void finish();

   private:
    void runHeartbeat();
    void stopHeartbeat();
    void renderLocked(std::chrono::steady_clock::time_point now);
    void closeLineLocked();

    int totalEvents_ = 0;
    int completedEvents_ = 0;
    int activityFrame_ = 0;
    int lastRenderedPercent_ = -1;
    std::size_t lastRenderedWidth_ = 0U;
    std::ostream *output_ = nullptr;
    std::chrono::steady_clock::time_point startTime_{};
    std::mutex mutex_;
    std::condition_variable heartbeatWake_;
    std::thread heartbeatThread_;
    bool enabled_ = false;
    bool drawn_ = false;
    bool lineClosed_ = false;
    bool stopRequested_ = false;
  };

  void printGenerateUsage(const char *programName);
  [[nodiscard]] RunOptions parseRunOptions(int argc, char **argv, bool &showHelp);

}  // namespace blastwave::app
