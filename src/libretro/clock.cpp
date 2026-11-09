// SPDX-License-Identifier: MIT
#include "clock.h"

namespace scp {
namespace {
uint64_t g_ms = 0;
uint64_t g_frac_us = 0;
unsigned g_rng = 1u;
}  // namespace

void clock_reset(uint64_t start_ms) {
  g_ms = start_ms;
  g_frac_us = 0;
}

void clock_advance_ms(double ms) {
  if (ms <= 0) return;
  const double total_us = ms * 1000.0 + static_cast<double>(g_frac_us);
  const auto whole_us = static_cast<uint64_t>(total_us);
  g_frac_us = static_cast<uint64_t>(total_us - static_cast<double>(whole_us));
  g_ms += whole_us / 1000ull;
  g_frac_us += whole_us % 1000ull;
  if (g_frac_us >= 1000ull) {
    g_ms += g_frac_us / 1000ull;
    g_frac_us %= 1000ull;
  }
}

uint64_t clock_ms() { return g_ms; }
uint64_t clock_us() { return g_ms * 1000ull + g_frac_us; }

void rng_seed(unsigned seed) { g_rng = seed ? seed : 1u; }

int rng_rand() {
  // Minimal LCG (same constants as glibc).
  g_rng = g_rng * 1103515245u + 12345u;
  return static_cast<int>((g_rng / 65536u) % 32768u);
}

}  // namespace scp
