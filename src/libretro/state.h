// SPDX-License-Identifier: MIT
#pragma once

#include "persist.h"

#include <cstddef>
#include <cstdint>

namespace scp {

// Fixed upper bound reported by retro_serialize_size (stable across frames).
constexpr std::size_t kStateMaxSize = 48u * 1024u;
constexpr uint16_t kStateVersion = 1;
constexpr char kStateMagic[4] = {'S', 'C', 'P', 'S'};
constexpr uint32_t kStateBuildId = 0x00080001u;  // 0.8.0-ish tag

// Returns bytes that would be written (0 if game not ready / too small).
std::size_t state_serialize_size();

bool state_serialize(void* data, std::size_t size);
bool state_unserialize(const void* data, std::size_t size);

// Pure header helper for unit tests (no engine). Layout: magic4|ver2|flags2|build4|…|crc4
inline bool state_header_valid(const uint8_t* data, std::size_t size,
                               std::size_t* payload_out = nullptr) {
  if (!data || size < 16) return false;
  if (data[0] != kStateMagic[0] || data[1] != kStateMagic[1] ||
      data[2] != kStateMagic[2] || data[3] != kStateMagic[3])
    return false;
  const uint16_t ver = static_cast<uint16_t>(data[4] | (data[5] << 8));
  if (ver != kStateVersion) return false;
  const uint32_t build = static_cast<uint32_t>(data[8] | (data[9] << 8) |
                                               (data[10] << 16) | (data[11] << 24));
  if (build != kStateBuildId) return false;
  const uint32_t expect = crc32_bytes(data, size - 4);
  const uint32_t got = static_cast<uint32_t>(data[size - 4] | (data[size - 3] << 8) |
                                             (data[size - 2] << 16) | (data[size - 1] << 24));
  if (expect != got) return false;
  if (payload_out) *payload_out = size - 16;
  return true;
}

}  // namespace scp
