// SPDX-License-Identifier: MIT
#include "persist.h"

#include <cstring>

namespace scp {
namespace {

alignas(8) uint8_t g_save_ram[kSaveRamSize]{};

inline void write_u16_le(uint8_t* p, uint16_t v) {
  p[0] = static_cast<uint8_t>(v & 0xFF);
  p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
}

inline void write_u32_le(uint8_t* p, uint32_t v) {
  p[0] = static_cast<uint8_t>(v & 0xFF);
  p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
  p[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
  p[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}

inline void write_i32_le(uint8_t* p, int32_t v) {
  write_u32_le(p, static_cast<uint32_t>(v));
}

inline uint16_t read_u16_le(const uint8_t* p) {
  return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

inline uint32_t read_u32_le(const uint8_t* p) {
  return static_cast<uint32_t>(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

inline int32_t read_i32_le(const uint8_t* p) {
  return static_cast<int32_t>(read_u32_le(p));
}

}  // namespace

uint32_t crc32_bytes(const uint8_t* data, std::size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (std::size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int b = 0; b < 8; ++b) {
      const uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

void* save_ram_data() { return g_save_ram; }
std::size_t save_ram_size() { return kSaveRamSize; }

void save_ram_clear() { std::memset(g_save_ram, 0, sizeof(g_save_ram)); }

bool save_ram_pack(const HighScoreTableView& table, uint8_t* dst, std::size_t dst_size) {
  if (!dst || dst_size < kSaveRamSize) return false;
  std::memset(dst, 0, kSaveRamSize);
  dst[0] = 'S';
  dst[1] = 'C';
  dst[2] = 'P';
  dst[3] = 'H';
  write_u16_le(dst + 4, kSaveRamVersion);
  write_u16_le(dst + 6, 0);
  for (int i = 0; i < 5; ++i) {
    uint8_t* e = dst + 8 + i * 36;
    std::memcpy(e, table.entries[i].name, 32);
    e[31] = 0;
    write_i32_le(e + 32, table.entries[i].score);
  }
  const uint32_t crc = crc32_bytes(dst, kSaveRamSize - 4);
  write_u32_le(dst + (kSaveRamSize - 4), crc);
  return true;
}

bool save_ram_unpack(const uint8_t* src, std::size_t src_size, HighScoreTableView& table) {
  if (!src || src_size < kSaveRamSize) return false;
  if (src[0] != 'S' || src[1] != 'C' || src[2] != 'P' || src[3] != 'H') return false;
  if (read_u16_le(src + 4) != kSaveRamVersion) return false;
  const uint32_t expect = crc32_bytes(src, kSaveRamSize - 4);
  const uint32_t got = read_u32_le(src + (kSaveRamSize - 4));
  if (expect != got) return false;

  std::memset(&table, 0, sizeof(table));
  for (int i = 0; i < 5; ++i) {
    const uint8_t* e = src + 8 + i * 36;
    std::memcpy(table.entries[i].name, e, 32);
    table.entries[i].name[31] = 0;
    table.entries[i].score = read_i32_le(e + 32);
  }
  return true;
}

bool save_ram_store(const HighScoreTableView& table) {
  return save_ram_pack(table, g_save_ram, kSaveRamSize);
}

bool save_ram_load(HighScoreTableView& table) {
  return save_ram_unpack(g_save_ram, kSaveRamSize, table);
}

}  // namespace scp
