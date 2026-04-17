#pragma once

#include <iosfwd>
#include <string>

namespace blastwave::io {

  // Ensure the parent directory of a file output path exists before ROOT opens
  // the target for writing.
  void ensureOutputDirectoryExists(const std::string &outputPath, std::ostream &noticeStream);

}  // namespace blastwave::io
