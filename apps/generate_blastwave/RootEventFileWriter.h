#pragma once

#include <memory>
#include <string>

#include "blastwave/BlastWaveGenerator.h"
#include "generate_blastwave/RunOptions.h"

namespace blastwave::app {

  // Own the ROOT output contract and embedded QA payload for generated events
  // so the executable entrypoint only orchestrates runtime flow.
  class RootEventFileWriter {
   public:
    explicit RootEventFileWriter(const blastwave::app::RunOptions &runOptions);
    ~RootEventFileWriter();

    RootEventFileWriter(const RootEventFileWriter &) = delete;
    RootEventFileWriter &operator=(const RootEventFileWriter &) = delete;
    RootEventFileWriter(RootEventFileWriter &&) noexcept;
    RootEventFileWriter &operator=(RootEventFileWriter &&) noexcept;

    void writeEvent(const blastwave::GeneratedEvent &event);
    void finish();

   private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
  };

}  // namespace blastwave::app
