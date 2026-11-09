// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <cstdint>

namespace scp {

// Fixed SAVE_RAM layout (little-endian), exposed via retro_get_memory_*.
// magic[4]="SCPH" | ver u16 | reserved u16 | entry[5]{name[32], score i32} | crc32
constexpr std::size_t kSaveRamSize = 192;
constexpr uint16_t kSaveRamVersion = 1;

struct HighScoreEntryView {
  char name[32];
  int32_t score;
};

struct HighScoreTableView {
  HighScoreEntryView entries[5];
};

void* save_ram_data();
std::size_t save_ram_size();
void save_ram_clear();

bool save_ram_pack(const HighScoreTableView& table, uint8_t* dst, std::size_t dst_size);
bool save_ram_unpack(const uint8_t* src, std::size_t src_size, HighScoreTableView& table);

bool save_ram_store(const HighScoreTableView& table);
bool save_ram_load(HighScoreTableView& table);

uint32_t crc32_bytes(const uint8_t* data, std::size_t len);

}  // namespace scp
