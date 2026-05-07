#include <unistd.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "generate_blastwave/RunOptions.h"

namespace {

  constexpr int kBarWidth = 50;
  constexpr std::chrono::seconds kHeartbeatInterval(1);
  constexpr std::array<char, 4> kActivityFrames = {'/', '-', '\\', '|'};

  bool shouldEnableProgress(blastwave::app::ProgressMode progressMode) {
    switch (progressMode) {
      case blastwave::app::ProgressMode::Auto:
        return ::isatty(STDERR_FILENO) != 0;
      case blastwave::app::ProgressMode::Enabled:
        return true;
      case blastwave::app::ProgressMode::Disabled:
        return false;
    }

    return false;
  }

  int progressPercent(int completedEvents, int totalEvents) {
    if (totalEvents <= 0) {
      return 0;
    }

    const int clampedCompletedEvents = std::clamp(completedEvents, 0, totalEvents);
    return static_cast<int>((100LL * clampedCompletedEvents) / totalEvents);
  }

  std::string formatProgressBar(int percent) {
    std::string bar(static_cast<std::size_t>(kBarWidth), '-');
    if (percent >= 100) {
      std::fill(bar.begin(), bar.end(), '=');
      return bar;
    }

    const int headIndex = std::min((percent * kBarWidth) / 100, kBarWidth - 1);
    for (int index = 0; index < headIndex; ++index) {
      bar[static_cast<std::size_t>(index)] = '=';
    }
    bar[static_cast<std::size_t>(headIndex)] = '>';
    return bar;
  }

  char activityFrame(int frame) {
    const int nonNegativeFrame = std::max(frame, 0);
    const std::size_t frameIndex = static_cast<std::size_t>(nonNegativeFrame % static_cast<int>(kActivityFrames.size()));
    return kActivityFrames[frameIndex];
  }

  std::string formatPositiveEta(std::int64_t remainingSeconds) {
    if (remainingSeconds <= 0) {
      return "00:00";
    }

    const std::int64_t hours = remainingSeconds / 3600;
    const std::int64_t minutes = (remainingSeconds % 3600) / 60;
    const std::int64_t seconds = remainingSeconds % 60;

    std::ostringstream output;
    output << std::setfill('0') << std::setw(2) << hours << ':' << std::setw(2) << minutes << ':' << std::setw(2) << seconds;
    return output.str();
  }

  // Estimate remaining wall time from the mean completed-event duration. This
  // deliberately avoids smoothing state so heartbeat redraws stay cheap.
  std::string formatEta(const blastwave::app::ProgressRenderSnapshot &snapshot) {
    if (snapshot.totalEvents <= 0) {
      return "--:--";
    }

    const int completedEvents = std::clamp(snapshot.completedEvents, 0, snapshot.totalEvents);
    if (completedEvents <= 0) {
      return "--:--";
    }
    if (completedEvents >= snapshot.totalEvents) {
      return "00:00";
    }

    const std::int64_t elapsedMillis = std::max<std::int64_t>(1, std::chrono::duration_cast<std::chrono::milliseconds>(snapshot.elapsed).count());
    const std::int64_t remainingEvents = static_cast<std::int64_t>(snapshot.totalEvents - completedEvents);
    const std::int64_t completedEvents64 = static_cast<std::int64_t>(completedEvents);
    const std::int64_t remainingMillis = (elapsedMillis * remainingEvents + completedEvents64 - 1) / completedEvents64;
    const std::int64_t remainingSeconds = (remainingMillis + 999) / 1000;
    return formatPositiveEta(remainingSeconds);
  }

}  // namespace

namespace blastwave::app {

  // Build the full carriage-return line from value data only; live reporting
  // wraps this helper with timing, locking, and terminal output.
  std::string formatProgressLine(const ProgressRenderSnapshot &snapshot) {
    const int percent = progressPercent(snapshot.completedEvents, snapshot.totalEvents);
    std::ostringstream output;
    output << '[' << formatProgressBar(percent) << "] " << percent << "% | " << activityFrame(snapshot.activityFrame) << " | ETA " << formatEta(snapshot);
    return output.str();
  }

  ProgressReporter::ProgressReporter(int totalEvents, ProgressMode progressMode)
      : totalEvents_(totalEvents), output_(&std::cerr), startTime_(std::chrono::steady_clock::now()), enabled_(totalEvents > 0 && shouldEnableProgress(progressMode)) {
    if (enabled_) {
      heartbeatThread_ = std::thread(&ProgressReporter::runHeartbeat, this);
    }
  }

  ProgressReporter::~ProgressReporter() {
    stopHeartbeat();

    std::lock_guard<std::mutex> lock(mutex_);
    closeLineLocked();
  }

  // The heartbeat redraws the latest completed-event state once per second so
  // long single-event work still leaves visible liveness on stderr.
  void ProgressReporter::runHeartbeat() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!stopRequested_) {
      heartbeatWake_.wait_for(lock, kHeartbeatInterval, [this]() {
        return stopRequested_;
      });
      if (stopRequested_) {
        break;
      }

      ++activityFrame_;
      renderLocked(std::chrono::steady_clock::now());
    }
  }

  void ProgressReporter::stopHeartbeat() {
    if (!enabled_) {
      return;
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopRequested_ = true;
    }
    heartbeatWake_.notify_all();

    if (heartbeatThread_.joinable()) {
      heartbeatThread_.join();
    }
  }

  // Event-loop updates record every completed-event count but only redraw when
  // the integer percentage changes; the heartbeat owns same-percent liveness.
  void ProgressReporter::update(int completedEvents) {
    if (!enabled_) {
      return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    completedEvents_ = std::clamp(completedEvents, 0, totalEvents_);

    const int percent = progressPercent(completedEvents_, totalEvents_);
    if (drawn_ && percent == lastRenderedPercent_) {
      return;
    }

    renderLocked(std::chrono::steady_clock::now());
  }

  void ProgressReporter::finish() {
    if (!enabled_) {
      return;
    }

    stopHeartbeat();

    std::lock_guard<std::mutex> lock(mutex_);
    completedEvents_ = totalEvents_;
    renderLocked(std::chrono::steady_clock::now());
    closeLineLocked();
  }

  // Rendering is serialized with state updates and pads shorter lines so old
  // ETA text does not linger after carriage-return redraws.
  void ProgressReporter::renderLocked(std::chrono::steady_clock::time_point now) {
    const ProgressRenderSnapshot snapshot{totalEvents_, completedEvents_, activityFrame_, now - startTime_};
    const std::string line = formatProgressLine(snapshot);

    (*output_) << '\r' << line;
    if (line.size() < lastRenderedWidth_) {
      (*output_) << std::string(lastRenderedWidth_ - line.size(), ' ');
    }
    (*output_) << std::flush;

    drawn_ = true;
    lineClosed_ = false;
    lastRenderedPercent_ = progressPercent(completedEvents_, totalEvents_);
    lastRenderedWidth_ = line.size();
  }

  void ProgressReporter::closeLineLocked() {
    if (drawn_ && !lineClosed_) {
      (*output_) << '\n' << std::flush;
      lineClosed_ = true;
    }
  }

}  // namespace blastwave::app
