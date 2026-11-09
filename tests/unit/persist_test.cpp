// SPDX-License-Identifier: MIT
#include "persist.h"

#include <cstdio>
#include <cstring>

static int failures = 0;

static void expect(bool cond, const char* msg) {
  if (!cond) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    ++failures;
  }
}

int main() {
  expect(scp::kSaveRamSize == 192, "save size");

  scp::HighScoreTableView table{};
  std::strncpy(table.entries[0].name, "ACE", 31);
  table.entries[0].score = 1234567;
  std::strncpy(table.entries[1].name, "BOB", 31);
  table.entries[1].score = 42;
  for (int i = 2; i < 5; ++i) table.entries[i].score = -999;

  uint8_t buf[scp::kSaveRamSize];
  expect(scp::save_ram_pack(table, buf, sizeof(buf)), "pack");
  expect(std::memcmp(buf, "SCPH", 4) == 0, "magic");

  scp::HighScoreTableView out{};
  expect(scp::save_ram_unpack(buf, sizeof(buf), out), "unpack");
  expect(std::strcmp(out.entries[0].name, "ACE") == 0, "name0");
  expect(out.entries[0].score == 1234567, "score0");
  expect(std::strcmp(out.entries[1].name, "BOB") == 0, "name1");
  expect(out.entries[1].score == 42, "score1");

  buf[scp::kSaveRamSize - 1] ^= 0xFF;
  expect(!scp::save_ram_unpack(buf, sizeof(buf), out), "bad crc");

  scp::save_ram_clear();
  expect(scp::save_ram_store(table), "store");
  scp::HighScoreTableView loaded{};
  expect(scp::save_ram_load(loaded), "load");
  expect(loaded.entries[0].score == 1234567, "loaded score");

  scp::HighScoreTableView empty{};
  uint8_t a[scp::kSaveRamSize], b[scp::kSaveRamSize];
  scp::save_ram_pack(empty, a, sizeof(a));
  scp::save_ram_pack(empty, b, sizeof(b));
  expect(std::memcmp(a, b, sizeof(a)) == 0, "deterministic pack");

  if (failures) {
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
  }
  std::puts("persist_unit ok");
  return 0;
}
