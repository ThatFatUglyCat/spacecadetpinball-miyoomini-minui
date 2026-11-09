// SPDX-License-Identifier: MIT
#include "content.h"

#include <cstdio>
#include <cstring>
#include <string>

static int failures = 0;

static void expect(bool cond, const char* msg) {
  if (!cond) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    ++failures;
  }
}

int main() {
  {
    scp::ContentCheck c = scp::validate_dat(nullptr);
    expect(c.status == scp::ContentStatus::MissingPath, "null path");
  }
  {
    scp::ContentCheck c = scp::validate_dat("/tmp/scp-unit-missing-dat-xyz.dat");
    expect(c.status == scp::ContentStatus::NotFound, "missing file");
  }

  const char* path = "/tmp/scp-unit-synthetic.dat";
  {
    FILE* f = std::fopen(path, "wb");
    expect(f != nullptr, "create synthetic");
    if (f) {
      const char magic[] = "PARTOUT(4.0)RESOURCE";
      std::fwrite(magic, 1, sizeof(magic) - 1, f);
      char pad[128];
      std::memset(pad, 0, sizeof(pad));
      std::fwrite(pad, 1, sizeof(pad), f);
      std::fclose(f);
    }
    scp::ContentCheck c = scp::validate_dat(path);
    expect(c.status == scp::ContentStatus::Ok, "good synthetic magic");
    std::remove(path);
  }

  {
    FILE* f = std::fopen(path, "wb");
    expect(f != nullptr, "create bad synthetic");
    if (f) {
      const char bad[] = "NOT-A-VALID-HEADER!!!!!";
      std::fwrite(bad, 1, sizeof(bad) - 1, f);
      char pad[128];
      std::memset(pad, 0, sizeof(pad));
      std::fwrite(pad, 1, sizeof(pad), f);
      std::fclose(f);
    }
    scp::ContentCheck c = scp::validate_dat(path);
    expect(c.status == scp::ContentStatus::BadMagic, "bad magic");
    std::remove(path);
  }

  if (failures) {
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
  }
  std::puts("CONTENT UNIT OK");
  return 0;
}
