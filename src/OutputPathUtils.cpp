#include "blastwave/io/OutputPathUtils.h"

#include <filesystem>
#include <ostream>
#include <stdexcept>
#include <system_error>

namespace blastwave::io {

  void ensureOutputDirectoryExists(const std::string &outputPath, std::ostream &noticeStream) {
    const std::filesystem::path targetPath(outputPath);
    const std::filesystem::path parentDirectory = targetPath.parent_path();
    if (parentDirectory.empty()) {
      return;
    }

    std::error_code statusError;
    const bool parentExists = std::filesystem::exists(parentDirectory, statusError);
    if (statusError) {
      throw std::runtime_error("Failed to inspect output directory '" + parentDirectory.string() + "': " + statusError.message());
    }

    if (parentExists) {
      if (!std::filesystem::is_directory(parentDirectory, statusError)) {
        const std::string errorSuffix = statusError ? ": " + statusError.message() : "";
        throw std::runtime_error("Output path parent exists but is not a directory: " + parentDirectory.string() + errorSuffix);
      }
      return;
    }

    // Tell the caller why output continues instead of failing on a missing
    // parent directory.
    noticeStream << "Output directory does not exist. Creating: " << parentDirectory.string() << '\n';

    std::error_code createError;
    std::filesystem::create_directories(parentDirectory, createError);
    if (createError) {
      throw std::runtime_error("Failed to create output directory '" + parentDirectory.string() + "': " + createError.message());
    }
  }

}  // namespace blastwave::io
