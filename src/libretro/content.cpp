// SPDX-License-Identifier: MIT
#include "content.h"

#include <cstdio>
#include <cstring>

namespace scp {
namespace {

constexpr const char kMagic[] = "PARTOUT(4.0)RESOURCE";
constexpr std::size_t kMinDatBytes = 64;

}  // namespace

const char* status_label(ContentStatus status) {
  switch (status) {
    case ContentStatus::Ok: return "ok";
    case ContentStatus::MissingPath: return "missing path";
    case ContentStatus::NotFound: return "file not found";
    case ContentStatus::TooSmall: return "file too small";
    case ContentStatus::BadMagic: return "invalid DAT magic";
    case ContentStatus::IoError: return "I/O error";
  }
  return "unknown";
}

ContentCheck validate_dat(const char* path) {
  ContentCheck out;
  if (!path || !path[0]) {
    out.status = ContentStatus::MissingPath;
    out.message = "No content path. Place PINBALL.DAT in the ROM folder.";
    return out;
  }

  FILE* f = std::fopen(path, "rb");
  if (!f) {
    out.status = ContentStatus::NotFound;
    out.message = "PINBALL.DAT not found. Copy your legitimate game data next to the ROM entry.";
    return out;
  }

  if (std::fseek(f, 0, SEEK_END) != 0) {
    std::fclose(f);
    out.status = ContentStatus::IoError;
    out.message = "Could not seek in DAT file.";
    return out;
  }
  long size = std::ftell(f);
  if (size < 0) {
    std::fclose(f);
    out.status = ContentStatus::IoError;
    out.message = "Could not measure DAT file.";
    return out;
  }
  if (static_cast<std::size_t>(size) < kMinDatBytes) {
    std::fclose(f);
    out.status = ContentStatus::TooSmall;
    out.message = "DAT file is too small to be valid.";
    return out;
  }

  if (std::fseek(f, 0, SEEK_SET) != 0) {
    std::fclose(f);
    out.status = ContentStatus::IoError;
    out.message = "Could not rewind DAT file.";
    return out;
  }

  char magic[sizeof(kMagic)];
  std::memset(magic, 0, sizeof(magic));
  const std::size_t want = sizeof(kMagic) - 1;
  if (std::fread(magic, 1, want, f) != want) {
    std::fclose(f);
    out.status = ContentStatus::IoError;
    out.message = "Could not read DAT header.";
    return out;
  }
  std::fclose(f);

  if (std::memcmp(magic, kMagic, want) != 0) {
    out.status = ContentStatus::BadMagic;
    out.message = "File is not a Space Cadet DAT (bad header).";
    return out;
  }

  out.status = ContentStatus::Ok;
  out.message = "DAT looks valid.";
  return out;
}

}  // namespace scp
