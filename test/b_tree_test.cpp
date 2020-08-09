#include <gtest/gtest.h>

#include "b_tree.hpp"
#include "rng_utility.hpp"

TEST(BTreeTest, Constructor)
{
    constexpr std::size_t B = 8;
    constexpr std::size_t M = 40;
    constexpr std::size_t K = 3;
    const std::size_t N     = 15;
    auto vs                 = rng.vec<int>(N, -100, 100);
    std::sort(vs.begin(), vs.end());
    vs.erase(std::unique(vs.begin(), vs.end()), vs.end());
    b_tree<int, B, M, K> btree(vs);
    ASSERT_EQ(btree.PageSize, B);
    ASSERT_EQ(btree.CacheSize, M);
    ASSERT_EQ(btree.NodeKeyNum, K);
    ASSERT_EQ(btree.statistic().disk_read_count, 0);
    ASSERT_EQ(btree.statistic().disk_write_count, 0);
}
TEST(BTreeTest, LowerBound)
{
    constexpr std::size_t B = 8;
    constexpr std::size_t M = 40;
    constexpr std::size_t K = 3;
    const std::size_t N     = 100;
    auto vs                 = rng.vec<int>(N, -100, 100);
    b_tree<int, B, M, K> btree(vs);
    std::sort(vs.begin(), vs.end());
    vs.erase(std::unique(vs.begin(), vs.end()), vs.end());
    const std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const int key         = rng.val<int>(-100, 100);
        const auto it         = std::lower_bound(vs.begin(), vs.end(), key);
        const auto [found, x] = btree.lower_bound(key);
        if (it == vs.end()) {
            ASSERT_FALSE(found);
            ASSERT_EQ(x, key);
        } else {
            ASSERT_TRUE(found);
            ASSERT_EQ(*it, x);
        }
    }
}
