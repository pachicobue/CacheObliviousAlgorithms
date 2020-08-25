#include <gtest/gtest.h>

#include "common/rng.hpp"

namespace {
using data_t  = int;
uint64_t seed = 20200822;
data_t min    = -1000;
data_t max    = 1000;
}  // namespace

TEST(RngTest, GenValue)
{
    rng_base rng{seed};
    constexpr std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const data_t x = rng.val<data_t>(min, max);
        ASSERT_TRUE(min <= x);
        ASSERT_TRUE(x <= max);
    }
}

TEST(RngTest, GenVec)
{
    rng_base rng{seed};
    constexpr std::size_t N = 100;
    constexpr std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const auto xs = rng.vec<data_t>(N, min, max);
        ASSERT_EQ(xs.size(), N);
        for (const auto x : xs) {
            ASSERT_TRUE(min <= x);
            ASSERT_TRUE(x <= max);
        }
    }
}
