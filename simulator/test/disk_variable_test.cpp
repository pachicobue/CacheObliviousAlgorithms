#include <gtest/gtest.h>

#include "common/rng.hpp"
#include "simulator/disk_variable.hpp"

namespace {
constexpr uint64_t seed = 20200826;
}

TEST(DiskVariableTest, Int)
{
    using data_t = int;
    rng_base rng{seed};
    disk_var<data_t> dv;
    data_t v         = rng.val(-100, 100);
    dv.illegal_ref() = v;

    constexpr std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type = rng.val(0, 1);
        if (type == 0) {
            const data_t actual = v;
            const data_t ans    = dv.illegal_ref();
            ASSERT_EQ(ans, actual);
        } else {
            v                = rng.val(-100, 100);
            dv.illegal_ref() = v;
        }
    }
}

TEST(DiskVariableTest, Struct)
{
    rng_base rng{seed};
    struct data_t
    {
        int a       = 0;
        long long b = 0;
    };
    auto rand = [&]() {
        return data_t{rng.val<int>(-100, 100), rng.val<long long>(-1000000, 1000000)};
    };
    disk_var<data_t> dv;
    data_t v         = rand();
    dv.illegal_ref() = v;

    constexpr std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type = rng.val(0, 1);
        if (type == 0) {
            const data_t actual = v;
            const data_t ans    = dv.illegal_ref();
            ASSERT_EQ(ans.a, actual.a);
            ASSERT_EQ(ans.b, actual.b);
        } else {
            v                = rand();
            dv.illegal_ref() = v;
        }
    }
}

TEST(DiskVariableTest, Array)
{
    using data_t            = int;
    constexpr std::size_t N = 100;
    rng_base rng{seed};
    std::array<data_t, N> vs;
    disk_var<std::array<data_t, N>> dvs;
    for (std::size_t i = 0; i < N; i++) {
        vs[i]                = rng.val(-100, 100);
        dvs.illegal_ref()[i] = vs[i];
    }
    constexpr std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val(0, 1);
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        if (type == 0) {
            const data_t actual = vs[index];
            const data_t ans    = dvs.illegal_ref()[index];
            ASSERT_EQ(ans, actual);
        } else {
            vs[index]                = rng.val(-100, 100);
            dvs.illegal_ref()[index] = vs[index];
        }
    }
}

TEST(DiskTest, DiskVariable_Vector)
{
    using data_t            = int;
    constexpr std::size_t N = 100;
    rng_base rng{seed};
    std::vector<data_t> vs(N);
    std::vector<disk_var<data_t>> dvs(N);
    for (std::size_t i = 0; i < N; i++) {
        vs[i]                = rng.val(-100, 100);
        dvs[i].illegal_ref() = vs[i];
    }
    constexpr std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val(0, 1);
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        if (type == 0) {
            const data_t actual = vs[index];
            const data_t ans    = dvs[index].illegal_ref();
            ASSERT_EQ(ans, actual);
        } else {
            vs[index]                = rng.val(-100, 100);
            dvs[index].illegal_ref() = vs[index];
        }
    }
}

TEST(DiskTest, DiskVector_Vector)
{
    using data_t            = int;
    constexpr std::size_t N = 100;
    rng_base rng{seed};
    std::vector<std::vector<int>> vss(N, std::vector<int>(N));
    std::vector<std::vector<disk_var<int>>> dvss(N, std::vector<disk_var<int>>(N));
    for (std::size_t i = 0; i < N; i++) {
        for (std::size_t j = 0; j < N; j++) {
            vss[i][j]                = rng.val(-100, 100);
            dvss[i][j].illegal_ref() = vss[i][j];
        }
    }
    constexpr std::size_t T = 1000;
    for (std::size_t t = 0; t < T; t++) {
        const std::size_t type  = rng.val(0, 1);
        const std::size_t index = rng.val<std::size_t>(0, N - 1);
        const std::size_t jndex = rng.val<std::size_t>(0, N - 1);
        if (type == 0) {
            const data_t actual = vss[index][jndex];
            const data_t ans    = dvss[index][jndex].illegal_ref();
            ASSERT_EQ(ans, actual);
        } else {
            vss[index][jndex]                = rng.val(-100, 100);
            dvss[index][jndex].illegal_ref() = vss[index][jndex];
        }
    }
}
