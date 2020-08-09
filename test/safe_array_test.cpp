#include <gtest/gtest.h>

#include "data_cache.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"

TEST(SafeArrayTest, Constructor)
{
    constexpr std::size_t B = 16;
    const std::size_t N     = 100;
    safe_array<int, B> as(N);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(&as[0]) % B, 0);
    for (std::size_t i = 0; i < N; i++) { ASSERT_EQ(as[i], 0); }
    safe_array<int, B> bs(N, 100);
    for (std::size_t i = 0; i < N; i++) { ASSERT_EQ(bs[i], 100); }
}
TEST(SafeArrayTest, RandomAccess)
{
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
