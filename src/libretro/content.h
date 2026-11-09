// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <string>

namespace scp {

enum class ContentStatus {
  Ok = 0,
  MissingPath,
  NotFound,
  TooSmall,
  BadMagic,
  IoError,
};

struct ContentCheck {
  ContentStatus status = ContentStatus::MissingPath;
  std::string message;
};

// Validates a user-supplied DAT path. Does not keep the file open.
// Magic string matches upstream partman.cpp: "PARTOUT(4.0)RESOURCE"
ContentCheck validate_dat(const char* path);

const char* status_label(ContentStatus status);

}  // namespace scp
