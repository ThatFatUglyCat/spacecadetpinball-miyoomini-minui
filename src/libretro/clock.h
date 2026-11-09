// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>

namespace scp {

// Deterministic clock used by the SDL shim and game step.
void clock_reset(uint64_t start_ms = 0);
void clock_advance_ms(double ms);
uint64_t clock_ms();
uint64_t clock_us();

void rng_seed(unsigned seed);
int rng_rand();  // replaces std::rand path via patch later if needed

}  // namespace scp
