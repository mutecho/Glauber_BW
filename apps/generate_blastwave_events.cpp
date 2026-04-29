#include <exception>
#include <iostream>

#include "blastwave/BlastWaveGenerator.h"
#include "generate_blastwave/RootEventFileWriter.h"
#include "generate_blastwave/RunOptions.h"

int main(int argc, char **argv) {
  try {
    bool showHelp = false;
    const blastwave::app::RunOptions runOptions = blastwave::app::parseRunOptions(argc, argv, showHelp);
    if (showHelp) {
      blastwave::app::printGenerateUsage(argv[0]);
      return 0;
    }

    const blastwave::BlastWaveConfig &config = runOptions.config;
    const std::string &outputPath = runOptions.outputPath;

    blastwave::BlastWaveGenerator generator(config);
    blastwave::app::ProgressReporter progressReporter(config.nEvents, runOptions.progressMode);
    blastwave::app::RootEventFileWriter writer(runOptions);

    // Keep the app entrypoint focused on orchestration: parse, generate, write.
    progressReporter.update(0);
    for (int eventId = 0; eventId < config.nEvents; ++eventId) {
      const blastwave::GeneratedEvent event = generator.generateEvent(eventId);
      writer.writeEvent(event);
      progressReporter.update(eventId + 1);
    }

    writer.finish();
    progressReporter.finish();
    std::cout << "Wrote " << config.nEvents << " events to " << outputPath << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "generate_blastwave_events failed: " << error.what() << '\n';
    return 1;
  }
}
