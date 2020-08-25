#include "rng.hpp"

rng_base::rng_base(const uint64_t seed) : rng(seed) {}

uint64_t rng_base::next(const uint64_t max)
{
    if (max == 0xFFFFFFFFFFFFFFFF) { return rng(); }
    uint64_t mask = 1;
    for (; mask < max + 1; mask <<= 1) {}
    mask--;
    while (true) {
        const uint64_t ans = rng() & mask;
        if (ans <= max) { return ans; }
    }
}
