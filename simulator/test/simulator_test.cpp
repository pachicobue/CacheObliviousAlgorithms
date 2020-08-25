#include <gtest/gtest.h>

#include "common/rng.hpp"
#include "simulator/simulator.hpp"

namespace {
constexpr uint64_t seed = 20200810;

struct Data
{
    int a                = 0;
    double b             = 0;
    unsigned long long c = 0;
    friend bool operator==(const Data& data1, const Data& data2) { return data1.a == data2.a and data1.b == data2.b and data1.c == data2.c; }
};
Data randomData()
{
    rng_base rng(seed);
    Data data;
    data.a = rng.val<int>(-10, 10);
    data.b = static_cast<double>(rng.val<int>(0, 100)) / 100;
    data.c = rng.val<unsigned long long>(0, 1ULL << 60);
    return data;
}
}  // anonymous namespace

TEST(SimulatorTest, Initialize)
{
    constexpr std::size_t B = 16;
    constexpr std::size_t M = 160;
    sim::initialize(B, M);
    ASSERT_EQ(sim::cache_miss_count().disk_read_count, 0);
    ASSERT_EQ(sim::cache_miss_count().disk_write_count, 0);
}

TEST(SimulatorTest, ReadWrite)
{
    rng_base rng(seed);
    constexpr std::size_t B = 16;
    constexpr std::size_t M = 160;
    sim::initialize(B, M);
    const std::size_t N = 100;
    std::vector<disk_var<Data>> datas(N);
    std::vector<Data> actuals(N);
    for (std::size_t i = 0; i < N; i++) {
        const Data data = randomData();
        actuals[i]      = data;
        sim::write(datas[i], data);
    }
    const std::size_t T = 10000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val<std::size_t>(0, 1);
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        if (type == 0) {
            const auto data   = sim::read(datas[index]);
            const auto actual = actuals[index];
            ASSERT_EQ(data, actual);
        } else {
            const auto data = randomData();
            sim::write(datas[index], data);
            actuals[index] = data;
        }
    }
    ASSERT_NE(sim::cache_miss_count().disk_read_count, 0);
    ASSERT_NE(sim::cache_miss_count().disk_write_count, 0);
}
