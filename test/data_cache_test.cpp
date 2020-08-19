#include <gtest/gtest.h>

#include "data_cache.hpp"
#include "rng_utility.hpp"

namespace {
constexpr uint64_t seed = 20190810;

struct Data
{
    int a                = 0;
    double b             = 0;
    unsigned long long c = 0;
    friend bool operator==(const Data& data1, const Data& data2) { return data1.a == data2.a and data1.b == data2.b and data1.c == data2.c; }
};
Data randomData()
{
    rng_base<std::mt19937> rng(seed);
    Data data;
    data.a = rng.val<int>(-10, 10);
    data.b = static_cast<double>(rng.val<int>(0, 100)) / 100;
    data.c = rng.val<unsigned long long>(0, 1ULL << 60);
    return data;
}
}  // anonymous namespace

TEST(DataCacheTest, Constructor)
{
    constexpr std::size_t B  = 8;
    constexpr std::size_t M_ = 42;
    constexpr std::size_t M  = (M_ + B - 1UL) / B * B;
    data_cache dcache(B, M_);
    ASSERT_EQ(dcache.PageSize, B);
    ASSERT_EQ(dcache.CacheSize, M);
    ASSERT_EQ(dcache.CacheLineNum, M / B);
    ASSERT_EQ(dcache.statistic().disk_read_count, 0);
    ASSERT_EQ(dcache.statistic().disk_write_count, 0);
}

TEST(DataCacheTest, ReadWrite)
{
    rng_base<std::mt19937> rng(seed);
    constexpr std::size_t B = 16;
    constexpr std::size_t M = 160;
    data_cache dcache(B, M);
    const std::size_t N = 100;
    disk_vector<Data> datas(N);
    std::vector<Data> actuals(N);
    for (std::size_t i = 0; i < N; i++) {
        const Data data = randomData();
        actuals[i]      = data;
        dcache.disk_write(datas.addr(i), data);
    }
    const std::size_t T = 10000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val<std::size_t>(0, 1);
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        if (type == 0) {
            const auto data   = dcache.disk_read<Data>(datas.addr(index));
            const auto actual = actuals[index];
            ASSERT_EQ(data, actual);
        } else {
            const auto data = randomData();
            dcache.disk_write(datas.addr(index), data);
            actuals[index] = data;
        }
    }
}

TEST(DataCacheTest, ReadWriteRaw)
{
    rng_base<std::mt19937> rng(seed);
    constexpr std::size_t B = 16;
    constexpr std::size_t M = 160;
    data_cache dcache(B, M);
    const std::size_t N = 100;
    disk_vector<Data> datas(N);
    std::vector<Data> actuals(N);
    for (std::size_t i = 0; i < N; i++) {
        const Data data = randomData();
        actuals[i]      = data;
        dcache.disk_write_raw(datas.addr(i), data);
    }
    const std::size_t T = 10000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val<std::size_t>(0, 1);
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        if (type == 0) {
            const auto data   = dcache.disk_read_raw<Data>(datas.addr(index));
            const auto actual = actuals[index];
            ASSERT_EQ(data, actual);
        } else {
            const auto data = randomData();
            dcache.disk_write_raw(datas.addr(index), data);
            actuals[index] = data;
        }
    }
    ASSERT_EQ(dcache.statistic().disk_read_count, 0);
    ASSERT_EQ(dcache.statistic().disk_write_count, 0);

    const auto data   = data_cache::disk_read_raw<Data>(datas.addr(0));
    const auto actual = actuals[0];
    ASSERT_EQ(data, actual);
}

TEST(DataCacheTest, Flush)
{
    rng_base<std::mt19937> rng(seed);
    constexpr std::size_t B = 16;
    constexpr std::size_t M = 160;
    data_cache dcache(B, M);
    const std::size_t N = 1;
    disk_vector<Data> datas(N);
    const Data data = randomData();
    dcache.disk_write(datas.addr(0), data);
    ASSERT_EQ(dcache.statistic().disk_write_count, 0);
    dcache.flush();
    ASSERT_NE(dcache.statistic().disk_write_count, 0);
}

TEST(DataCacheTest, Reset)
{
    rng_base<std::mt19937> rng(seed);
    constexpr std::size_t B = 16;
    constexpr std::size_t M = 160;
    data_cache dcache{B, M};
    const std::size_t N = 3;
    disk_vector<Data> datas(N);
    for (std::size_t i = 0; i < N; i++) { dcache.disk_write(datas.addr(i), randomData()); }
    dcache.reset();
    ASSERT_EQ(dcache.statistic().disk_read_count, 0);
    ASSERT_EQ(dcache.statistic().disk_write_count, 0);
    dcache.disk_write<Data>(datas.addr(0), randomData());
    dcache.flush();
    ASSERT_NE(dcache.statistic().disk_read_count, 0);
    ASSERT_NE(dcache.statistic().disk_write_count, 0);
}
