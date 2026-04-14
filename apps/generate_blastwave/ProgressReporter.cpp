#include "generate_blastwave/RunOptions.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <unistd.h>

namespace {

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

}  // namespace

namespace blastwave::app {

  ProgressReporter::ProgressReporter(int totalEvents, ProgressMode progressMode)
      : totalEvents_(totalEvents), enabled_(totalEvents > 0 && shouldEnableProgress(progressMode)) {}

  ProgressReporter::~ProgressReporter() {
    if (drawn_ && !lineClosed_) {
      std::cerr << '\n' << std::flush;
    }
  }

  // Redraw only when the integer percentage changes so CLI progress remains
  // responsive without dominating generator runtime.
  void ProgressReporter::update(int completedEvents) {
    if (!enabled_) {
      return;
    }

    const int clampedCompletedEvents = std::clamp(completedEvents, 0, totalEvents_);
    const int percent = totalEvents_ > 0 ? (100 * clampedCompletedEvents) / totalEvents_ : 0;
    if (percent == lastPercent_) {
      return;
    }

    std::string bar(static_cast<std::size_t>(kBarWidth), '-');
    if (percent >= 100) {
      std::fill(bar.begin(), bar.end(), '=');
    } else {
      const int headIndex = std::min((percent * kBarWidth) / 100, kBarWidth - 1);
      for (int index = 0; index < headIndex; ++index) {
        bar[static_cast<std::size_t>(index)] = '=';
      }
      bar[static_cast<std::size_t>(headIndex)] = '>';
    }

    std::cerr << '\r' << '[' << bar << "] " << percent << '%' << std::flush;
    drawn_ = true;
    lineClosed_ = false;
    lastPercent_ = percent;
  }

  void ProgressReporter::finish() {
    if (!enabled_) {
      return;
    }

    update(totalEvents_);
    if (drawn_ && !lineClosed_) {
      std::cerr << '\n' << std::flush;
      lineClosed_ = true;
    }
  }

}  // namespace blastwave::app
