#include <chrono>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "generate_blastwave/RunOptions.h"

namespace {

  void require(bool condition, const std::string &message) {
    if (!condition) {
      throw std::runtime_error(message);
    }
  }

  void requireContains(const std::string &text, const std::string &needle, const std::string &message) {
    require(text.find(needle) != std::string::npos, message + " text='" + text + "'");
  }

  void runUnknownEtaBeforeFirstEventTest() {
    const blastwave::app::ProgressRenderSnapshot snapshot{10, 0, 0, std::chrono::seconds(0)};
    const std::string line = blastwave::app::formatProgressLine(snapshot);

    requireContains(line, "[>-------------------------------------------------] 0%", "Initial progress line should show zero percent.");
    requireContains(line, "| / |", "Initial progress line should show an activity frame.");
    requireContains(line, "ETA --:--", "ETA should be unknown before the first completed event.");
  }

  void runFiniteEtaTest() {
    const blastwave::app::ProgressRenderSnapshot snapshot{5, 2, 0, std::chrono::seconds(120)};
    const std::string line = blastwave::app::formatProgressLine(snapshot);

    requireContains(line, "40%", "Partial progress line should use integer percent.");
    requireContains(line, "ETA 00:03:00", "Partial progress line should estimate remaining time from event average.");
  }

  void runActivityFrameChangesWithoutPercentChangeTest() {
    const blastwave::app::ProgressRenderSnapshot firstFrame{5, 2, 0, std::chrono::seconds(120)};
    const blastwave::app::ProgressRenderSnapshot secondFrame{5, 2, 1, std::chrono::seconds(121)};
    const std::string firstLine = blastwave::app::formatProgressLine(firstFrame);
    const std::string secondLine = blastwave::app::formatProgressLine(secondFrame);

    requireContains(firstLine, "40%", "First same-percent line should keep percent.");
    requireContains(secondLine, "40%", "Second same-percent line should keep percent.");
    requireContains(firstLine, "| / |", "First same-percent line should use the first activity frame.");
    requireContains(secondLine, "| - |", "Second same-percent line should use the next activity frame.");
    require(firstLine != secondLine, "Activity frame should change even when percent is unchanged.");
  }

  void runFinalEtaTest() {
    const blastwave::app::ProgressRenderSnapshot snapshot{5, 5, 2, std::chrono::seconds(125)};
    const std::string line = blastwave::app::formatProgressLine(snapshot);

    requireContains(line, "[==================================================] 100%", "Finished progress line should render a full bar.");
    requireContains(line, "ETA 00:00", "Finished progress line should show zero remaining time.");
  }

}  // namespace

int main() {
  try {
    runUnknownEtaBeforeFirstEventTest();
    runFiniteEtaTest();
    runActivityFrameChangesWithoutPercentChangeTest();
    runFinalEtaTest();
    std::cout << "Progress reporter tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Progress reporter tests failed: " << error.what() << '\n';
    return 1;
  }
}
