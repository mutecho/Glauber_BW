#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "blastwave/io/OutputPathUtils.h"

namespace {

  void require(bool condition, const std::string &message) {
    if (!condition) {
      throw std::runtime_error(message);
    }
  }

  void runCreatesMissingDirectoryTest() {
    const std::filesystem::path testRoot = std::filesystem::temp_directory_path() / "blastwave_output_path_utils_test";
    const std::filesystem::path nestedDirectory = testRoot / "nested" / "qa";
    const std::filesystem::path outputPath = nestedDirectory / "sample.root";

    // Start from a clean temporary tree so the helper has to create the target
    // output directory itself.
    std::error_code cleanupError;
    std::filesystem::remove_all(testRoot, cleanupError);

    std::ostringstream firstNotice;
    blastwave::io::ensureOutputDirectoryExists(outputPath.string(), firstNotice);
    require(std::filesystem::is_directory(nestedDirectory), "Missing output directory should be created.");
    require(firstNotice.str().find("Creating: " + nestedDirectory.string()) != std::string::npos,
            "Creating a missing directory should emit a notice.");

    std::ostringstream secondNotice;
    blastwave::io::ensureOutputDirectoryExists(outputPath.string(), secondNotice);
    require(secondNotice.str().empty(), "Existing output directories should not emit a second creation notice.");

    std::filesystem::remove_all(testRoot, cleanupError);
  }

}  // namespace

int main() {
  try {
    runCreatesMissingDirectoryTest();
    std::cout << "Output path utility tests passed.\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Output path utility tests failed: " << error.what() << '\n';
    return 1;
  }
}
