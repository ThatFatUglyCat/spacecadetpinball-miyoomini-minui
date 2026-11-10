// SPDX-License-Identifier: MIT
// Header/CRC validation without linking the pinball engine.
#include "persist.h"
#include "state.h"

#include <cstdio>
#include <cstring>
#include <vector>

static int failures = 0;

static void expect(bool cond, const char* msg) {
  if (!cond) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    ++failures;
  }
}

static std::vector<uint8_t> make_header_blob() {
  std::vector<uint8_t> buf(16, 0);
  buf[0] = 'S';
  buf[1] = 'C';
  buf[2] = 'P';
  buf[3] = 'S';
  buf[4] = static_cast<uint8_t>(scp::kStateVersion & 0xFF);
  buf[5] = static_cast<uint8_t>((scp::kStateVersion >> 8) & 0xFF);
  const uint32_t build = scp::kStateBuildId;
  buf[8] = static_cast<uint8_t>(build & 0xFF);
  buf[9] = static_cast<uint8_t>((build >> 8) & 0xFF);
  buf[10] = static_cast<uint8_t>((build >> 16) & 0xFF);
  buf[11] = static_cast<uint8_t>((build >> 24) & 0xFF);
  const uint32_t crc = scp::crc32_bytes(buf.data(), 12);
  buf[12] = static_cast<uint8_t>(crc & 0xFF);
  buf[13] = static_cast<uint8_t>((crc >> 8) & 0xFF);
  buf[14] = static_cast<uint8_t>((crc >> 16) & 0xFF);
  buf[15] = static_cast<uint8_t>((crc >> 24) & 0xFF);
  return buf;
}

int main() {
  expect(scp::kStateMaxSize == 48u * 1024u, "max size");

  auto ok = make_header_blob();
  std::size_t payload = 99;
  expect(scp::state_header_valid(ok.data(), ok.size(), &payload), "valid header");
  expect(payload == 0, "empty payload");

  auto bad_magic = ok;
  bad_magic[0] = 'X';
  expect(!scp::state_header_valid(bad_magic.data(), bad_magic.size()), "bad magic");

  auto bad_ver = ok;
  bad_ver[4] = 99;
  {
    const uint32_t crc = scp::crc32_bytes(bad_ver.data(), 12);
    bad_ver[12] = static_cast<uint8_t>(crc & 0xFF);
    bad_ver[13] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    bad_ver[14] = static_cast<uint8_t>((crc >> 16) & 0xFF);
    bad_ver[15] = static_cast<uint8_t>((crc >> 24) & 0xFF);
  }
  expect(!scp::state_header_valid(bad_ver.data(), bad_ver.size()), "bad version");

  auto bad_crc = ok;
  bad_crc[15] ^= 0xFF;
  expect(!scp::state_header_valid(bad_crc.data(), bad_crc.size()), "bad crc");

  if (failures) {
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
  }
  std::puts("state_unit ok");
  return 0;
}
