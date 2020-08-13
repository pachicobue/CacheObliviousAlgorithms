#include <gtest/gtest.h>

#include "data_cache.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"

namespace {
constexpr uint64_t seed = 20190810;
}
TEST(SafeArrayTest, Constructor)
{
    constexpr std::size_t B = 16;
    const std::size_t N     = 100;
    safe_array<int, B> as(N);
    ASSERT_EQ(as.size(), N);
    for (std::size_t i = 0; i < N; i++) { ASSERT_EQ(as[i], 0); }
    safe_array<int, B> bs(N, 100);
    ASSERT_EQ(bs.size(), N);
    for (std::size_t i = 0; i < N; i++) { ASSERT_EQ(bs[i], 100); }
    safe_array<int, B> cs;
    ASSERT_EQ(cs.size(), 0);
}
TEST(SafeArrayTest, RandomAccess)
{
    rng_base<std::mt19937> rng(seed);
    constexpr std::size_t B = 16;
    const std::size_t N     = 100;
    std::vector<int> actuals(N);
    safe_array<int, B> datas(N);
    for (std::size_t i = 0; i < N; i++) {
        const int v = rng.val<int>(-100, 100);
        actuals[i] = v, datas[i] = v;
    }
    const std::size_t T = 10000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val<std::size_t>(0, 1);
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        if (type == 0) {
            const auto data   = datas[index];
            const auto actual = actuals[index];
            ASSERT_EQ(data, actual);
        } else {
            const int v  = rng.val<int>(-100, 100);
            datas[index] = v, actuals[index] = v;
        }
    }
}

TEST(SafeArrayTest, PushBack)
{
    rng_base<std::mt19937> rng(seed);
    constexpr std::size_t B = 16;
    const std::size_t N     = 100;
    std::vector<int> actuals(N);
    safe_array<int, B> datas(N);
    for (std::size_t i = 0; i < N; i++) {
        const int v = rng.val<int>(-100, 100);
        actuals[i] = v, datas[i] = v;
    }
    const std::size_t T = 10000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val<std::size_t>(0, 2);
        const std::size_t index = rng.val<std::size_t>(0, actuals.size() - 1);
        if (type == 0) {
            const auto data   = datas[index];
            const auto actual = actuals[index];
            ASSERT_EQ(data, actual);
        } else if (type == 1) {
            const int v  = rng.val<int>(-100, 100);
            datas[index] = v, actuals[index] = v;
        } else {
            const int v = rng.val<int>(-100, 100);
            actuals.push_back(v);
            datas.push_back(v);
            ASSERT_EQ(actuals.size(), datas.size());
        }
    }
}
