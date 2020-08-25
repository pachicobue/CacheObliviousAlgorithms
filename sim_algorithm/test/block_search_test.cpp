#include <gtest/gtest.h>

#include "common/rng.hpp"
#include "sim_algorithm/block_search.hpp"
#include "simulator/simulator.hpp"

namespace {
constexpr uint64_t seed = 20200810;
}  // anonymous namespace

TEST(BlockSearchTest, LowerBound)
{
    rng_base rng(seed);
    constexpr std::size_t B = 100;
    constexpr std::size_t M = 20000;
    sim::initialize(B, M);
    constexpr std::size_t H = 4;
    constexpr std::size_t N = (1 << 10);
    constexpr std::size_t T = (1 << 10);
    auto vs                 = rng.vec(N, Min, Max);
    const block_search searcher(vs, H);
    std::sort(vs.begin(), vs.end());
    vs.push_back(Max + 1);
    for (std::size_t t = 0; t < T; t++) {
        const data_t qx     = t == 0 ? Min : t + 1 == T ? Max : rng.val<data_t>(Min, Max);
        const data_t ans    = searcher.lower_bound(qx);
        const data_t actual = *std::lower_bound(vs.begin(), vs.end(), qx);
        ASSERT_EQ(actual, ans);
    }
}
