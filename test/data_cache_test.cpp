#include <gtest/gtest.h>

#include "data_cache.hpp"
#include "output_utility.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"

namespace {
struct Data
{
    int a                = 10;
    double b             = 100;
    unsigned long long c = 1000;
    friend bool operator==(const Data& data1, const Data& data2) { return data1.a == data2.a and data1.b == data2.b and data1.c == data2.c; }
};
Data randomData()
{
    Data data;
    data.a = rng.val<int>(-10, 10);
    data.b = static_cast<double>(rng.val<int>(0, 100)) / 100;
    data.c = rng.val<unsigned long long>(0, 1ULL << 60);
    return data;
}
}  // anonymous namespace

TEST(DataCacheTest, Constructor_Valid)
{
    const std::size_t B = 8;
    const std::size_t M = 40;
    data_cache dcache(B, M);
    ASSERT_EQ(dcache.page_size(), B);
    ASSERT_EQ(dcache.cache_size(), M);
    ASSERT_EQ(dcache.cacheline_num(), M / B);
    ASSERT_EQ(dcache.statistic().disk_read_count, 0);
    ASSERT_EQ(dcache.statistic().disk_write_count, 0);
}
TEST(DataCacheTest, Constructor_Invalid)
{
    ASSERT_DEATH({
        const std::size_t B = 104;
        const std::size_t M = 0;
        data_cache dcache(B, M);
    },
                 "Assertion.*failed");
    ASSERT_DEATH({
        const std::size_t B = 0;
        const std::size_t M = 40;
        data_cache dcache(B, M);
    },
                 "Assertion.*failed");
    ASSERT_DEATH({
        const std::size_t B = 8;
        const std::size_t M = 20;
        data_cache dcache(B, M);
    },
                 "Assertion.*failed");
    ASSERT_DEATH({
        const std::size_t B = 3;
        const std::size_t M = 9;
        data_cache dcache(B, M);
    },
                 "Assertion.*failed");
}
TEST(DataCacheTest, Read_Sequential)
{
    const std::size_t B = 8;
    const std::size_t M = 120;
    data_cache dcache(B, M);
    const std::size_t N = 100;
    safe_array<Data> datas(B, N);
    for (std::size_t i = 0; i < N; i++) { datas[i] = randomData(); }
    const std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t index = t % N;
        const auto data         = dcache.disk_read<Data>(reinterpret_cast<uintptr_t>(&datas[index]));
        ASSERT_EQ(datas[index], data);
    }
}
TEST(DataCacheTest, Read_Random)
{
    const std::size_t B = 8;
    const std::size_t M = 120;
    data_cache dcache(B, M);
    const std::size_t N = 100;
    safe_array<Data> datas(B, N);
    for (std::size_t i = 0; i < N; i++) { datas[i] = randomData(); }
    const std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        const auto data         = dcache.disk_read<Data>(reinterpret_cast<uintptr_t>(&datas[index]));
        ASSERT_EQ(datas[index], data);
    }
}
TEST(DataCacheTest, Write_Sequential)
{
    const std::size_t B = 16;
    const std::size_t M = 160;
    data_cache dcache(B, M);
    const std::size_t N = 100;
    safe_array<Data> datas(B, N);
    safe_array<Data> dests(B, N);
    for (std::size_t i = 0; i < N; i++) { datas[i] = randomData(); }
    const std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t index = t % N;
        dcache.disk_write(reinterpret_cast<uintptr_t>(&dests[index]), datas[index]);
        const auto data = dcache.disk_read<Data>(reinterpret_cast<uintptr_t>(&dests[index]));
        ASSERT_EQ(datas[index], data);
    }
    dcache.flush();
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t index = t % N;
        const auto data         = dcache.disk_read<Data>(reinterpret_cast<uintptr_t>(&dests[index]));
        ASSERT_EQ(datas[index], data);
        ASSERT_EQ(datas[index], dests[index]);
    }
}

TEST(DataCacheTest, Write_Random)
{
    const std::size_t B = 16;
    const std::size_t M = 160;
    data_cache dcache(B, M);
    const std::size_t N = 100;
    safe_array<Data> datas(B, N);
    safe_array<Data> dests(B, N);
    for (std::size_t i = 0; i < N; i++) { datas[i] = randomData(); }
    const std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        dcache.disk_write(reinterpret_cast<uintptr_t>(&dests[index]), datas[index]);
    }
    dcache.flush();
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        const auto data         = dcache.disk_read<Data>(reinterpret_cast<uintptr_t>(&dests[index]));
        ASSERT_EQ(datas[index], data);
        ASSERT_EQ(datas[index], dests[index]);
    }
}

TEST(DataCacheTest, ReadWrite)
{
    const std::size_t B = 16;
    const std::size_t M = 160;
    data_cache dcache(B, M);
    const std::size_t N = 100;
    safe_array<Data> datas(B, N);
    std::vector<Data> actuals(N);
    for (std::size_t i = 0; i < N; i++) {
        const Data data = randomData();
        datas[i] = data, actuals[i] = data;
    }
    const std::size_t T = 10000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val<std::size_t>(0, 1);
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        if (type == 0) {
            const auto data   = dcache.disk_read<Data>(reinterpret_cast<uintptr_t>(&datas[index]));
            const auto actual = actuals[index];
            ASSERT_EQ(data, actual);
        } else {
            const auto data = randomData();
            dcache.disk_write(reinterpret_cast<uintptr_t>(&datas[index]), data);
            actuals[index] = data;
        }
    }
}
