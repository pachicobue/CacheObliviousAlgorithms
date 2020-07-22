#include <gtest/gtest.h>

#include "data_cache.hpp"
TEST(DataCacheTest, Constructor)
{
    const std::size_t B = 100;
    const std::size_t M = 20000;
    data_cache dcache(B, M);
    ASSERT_EQ(dcache.block_size(), B);
    ASSERT_EQ(dcache.cache_size(), M);
}
