#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

#include "common/bit.hpp"
#include "common/rng.hpp"
#include "common/stopwatch.hpp"

// {{{ Global
namespace {

using data_t         = uint64_t;
constexpr data_t Min = 1;
constexpr data_t Max = (data_t{1} << 60);

constexpr std::size_t N = (1 << 20);
data_t Datas[N];

constexpr std::size_t T = (1 << 20);
data_t Queries[T];

constexpr uint64_t Seed = 20201013;
rng_base Rng{Seed};

stopwatch SW;

void data_init()
{
    Datas[0] = Min - 1;
    for (std::size_t i = 1; i + 1 < N; i++) {
        Datas[i] = Rng.val<data_t>(Min, Max);
    }
    Datas[N - 1] = Max + 1;
    std::sort(Datas, Datas + N);

    for (std::size_t t = 0; t < T; t++) {
        Queries[t] = Rng.val<data_t>(Min, Max);
    }
}

inline std::size_t left(const std::size_t n)
{
    const std::size_t i = lsb(n) - 1;
    return n - (1UL << i);
}
inline std::size_t right(const std::size_t n)
{
    const std::size_t i = lsb(n) - 1;
    return n + (1UL << i);
}

}  // anonymous namespace

// }}}

// {{{ Binary
namespace binary {

data_t xs[N];

data_t Ans = 0;

void init()
{
    for (std::size_t i = 0; i < N; i++) {
        xs[i] = Datas[i];
    }
    //    std::sort(xs, xs + N);
}

inline data_t lower_bound(const data_t v)
{
    std::size_t inf = 0, sup = N - 1;
    while (sup > inf + 1) {
        const std::size_t mid = (inf + sup) / 2;
        const data_t x        = xs[mid];
        if (x == v) { return x; }
        (x < v ? inf : sup) = mid;
    }
    return xs[sup];
}

inline void test()
{
    init();
    SW.rap();
    for (std::size_t t = 0; t < T; t++) {
        Ans += lower_bound(Queries[t]);
    }
    const auto dur_ms = SW.rap<std::chrono::nanoseconds>();
    std::cout << dur_ms << " ns" << std::endl;
}

}  // namespace binary
// }}}

// {{{ Block
namespace block {

constexpr std::size_t H = 5;

constexpr std::size_t NN = ceil2(N + 1) - 1;
constexpr std::size_t R  = (NN + 1) / 2;
std::size_t orders[NN];
std::size_t poss[NN + 1];
struct node_t
{
    data_t x;
    std::size_t left  = -1;
    std::size_t right = -1;
};
node_t nodes[NN];
std::size_t root_index;  // Rの場所

data_t Ans = 0;

void build_orders(const std::size_t r, std::size_t& index)
{
    const std::size_t height = lsb(r) + 1;
    const std::size_t offset = r - (1UL << (height - 1));
    const std::size_t uh     = (height - 1) % H + 1;
    const std::size_t dh     = height - uh;
    for (std::size_t i = 1; i < (1UL << uh); i++) {
        orders[index++] = i * (1UL << dh) + offset;
    }
    if (dh != 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            build_orders((1UL << (dh - 1)) * (i * 2 - 1) + offset, index);
        }
    }
}

void init()
{
    std::size_t index = 0;
    build_orders(R, index);
    for (std::size_t i = 0; i < NN; i++) {
        poss[orders[i]] = i;
    }
    root_index = poss[R];
    for (std::size_t i = 0; i < NN; i++) {
        const std::size_t ord = orders[i];
        nodes[i].x            = ord > N ? Max + 1 : Datas[ord - 1];
        if ((ord & 1UL) == 0) {
            nodes[i].left  = poss[left(ord)];
            nodes[i].right = poss[right(ord)];
        }
    }
}

inline data_t lower_bound(const data_t v)
{
    data_t ans = Max + 1;
    for (std::size_t ind = root_index; ind != static_cast<std::size_t>(-1);) {
        const node_t& node = nodes[ind];
        const data_t x     = node.x;
        if (x == v) { return v; }
        if (x < v) {
            ind = node.right;
        } else {
            ans = std::min(ans, x);
            ind = node.left;
        }
    }
    return ans;
}

inline void test()
{
    init();
    SW.rap();
    for (std::size_t t = 0; t < T; t++) {
        Ans += lower_bound(Queries[t]);
    }
    const auto dur_ms = SW.rap<std::chrono::nanoseconds>();
    std::cout << dur_ms << " ns" << std::endl;
}

}  // namespace block
// }}}

// {{{ vEB
namespace vEB {

constexpr std::size_t NN = ceil2(N + 1) - 1;
constexpr std::size_t R  = (NN + 1) / 2;
std::size_t orders[NN];
std::size_t poss[NN + 1];
struct node_t
{
    data_t x;
    std::size_t left  = -1;
    std::size_t right = -1;
};
node_t nodes[NN];
std::size_t root_index;  // Rの場所

data_t Ans = 0;

void build_orders(const std::size_t r, std::size_t& index)
{
    if (r & 1UL) {
        orders[index++] = r;
        return;
    }
    const std::size_t height = lsb(r) + 1;
    const std::size_t offset = r - (1UL << (height - 1));
    const std::size_t uh     = height / 2;
    const std::size_t dh     = height - uh;
    std::size_t old_index    = index;
    build_orders(r / (1UL << dh), index);
    for (std::size_t i = old_index; i < index; i++) {
        orders[i] *= (1UL << dh);
    }
    if (dh > 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            build_orders((1UL << (dh - 1)) * (i * 2 - 1) + offset, index);
        }
    }
}

void init()
{
    std::size_t index = 0;
    build_orders(R, index);
    for (std::size_t i = 0; i < NN; i++) {
        poss[orders[i]] = i;
    }
    root_index = poss[R];
    for (std::size_t i = 0; i < NN; i++) {
        const std::size_t ord = orders[i];
        nodes[i].x            = ord > N ? Max + 1 : Datas[ord - 1];
        if ((ord & 1UL) == 0) {
            nodes[i].left  = poss[left(ord)];
            nodes[i].right = poss[right(ord)];
        }
    }
}

inline data_t lower_bound(const data_t v)
{
    data_t ans = Max + 1;
    for (std::size_t ind = root_index; ind != static_cast<std::size_t>(-1);) {
        const node_t& node = nodes[ind];
        const data_t x     = node.x;
        if (x == v) { return v; }
        if (x < v) {
            ind = node.right;
        } else {
            ans = std::min(ans, x);
            ind = node.left;
        }
    }
    return ans;
}

inline void test()
{
    init();
    SW.rap();
    for (std::size_t t = 0; t < T; t++) {
        Ans += lower_bound(Queries[t]);
    }
    const auto dur_ms = SW.rap<std::chrono::nanoseconds>();
    std::cout << dur_ms << " ns" << std::endl;
}

}  // namespace vEB
// }}}

int main()
{
    data_init();
    binary::test();
    block::test();
    vEB::test();
    assert(binary::Ans == block::Ans);
    assert(binary::Ans == vEB::Ans);
    return 0;
}
