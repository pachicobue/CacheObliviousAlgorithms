#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

#include "common/bit.hpp"
#include "common/rng.hpp"
#include "common/stopwatch.hpp"

constexpr uint64_t Seed = 20201013;
rng_base Rng{Seed};
stopwatch SW;

inline std::size_t left(const std::size_t n)  // 二分探索木で頂点 n の左にある頂点番号
{
    const std::size_t i = lsb(n) - 1;
    return n - (1UL << i);
}
inline std::size_t right(const std::size_t n)  // 二分探索木で頂点 n の右にある頂点番号
{
    const std::size_t i = lsb(n) - 1;
    return n + (1UL << i);
}

/**
 * データ列
 */
using data_t            = uint32_t;
constexpr data_t Inf    = std::numeric_limits<data_t>::max();  // ∞を表現するためだけの定数
constexpr data_t Min    = 1;
constexpr data_t Max    = Inf >> 1;
constexpr std::size_t N = (1 << 24) + 64;
data_t* Xs;

/**
 * 検索する値たち
 */
constexpr std::size_t Q = (1 << 24);
data_t* Ys;

void data_init()
{
    Xs = new data_t[N];
    for (std::size_t i = 0; i < N; i++) {
        Xs[i] = Rng.val<data_t>(Min, Max);
    }
    std::sort(Xs, Xs + N);  // ここでソートしてしまう
    Ys = new data_t[Q];
    for (std::size_t q = 0; q < Q; q++) {
        Ys[q] = Rng.val<data_t>(Min, Max);
    }
}

namespace sorting {

/**
 * メインメモリ上で保持するデータ
 * - xs:レイアウト
 */
data_t* xs;

void init()
{
    xs = new data_t[N + 1];
    for (std::size_t i = 0; i < N; i++) {
        xs[i] = Xs[i];
    }
    xs[N] = Inf;  // 末尾にInfを入れておく
}

void fin()
{
    delete[] xs;
}

/**
 * クエリ応答
 * - 二分探索するだけ
 */
inline data_t lower_bound(const data_t v)
{
    int inf = -1, sup = N;
    while (sup - inf > 1) {
        const std::size_t mid = (std::size_t)((inf + sup) / 2);
        const data_t x        = xs[mid];
        if (x == v) { return v; }
        (x < v ? inf : sup) = mid;
    }
    return xs[sup];
}

inline void test()
{
    data_t sum = 0;
    std::cout << "[Sol1] Sorting" << std::endl;
    SW.rap();
    for (std::size_t q = 0; q < Q; q++) {
        sum += lower_bound(Ys[q]);
    }
    const auto dur_ms = SW.rap<std::chrono::nanoseconds>();
    std::cout << "Query Total: " << dur_ms << " ns" << std::endl;
    std::cout << "Sum(for Debug): " << sum << std::endl;
    std::cout << std::endl;
}

}  // namespace sorting

namespace blocking {

constexpr std::size_t TN = ceil2(N + 1) - 1;  // x1~xNが含まれる完全二分探索木のサイズ
constexpr std::size_t R  = (TN + 1) / 2;      // 完全二分探索木の根

std::size_t* poss;  // 頂点番号iは、レイアウトでpos[i]番目に存在
std::size_t* inds;  // レイアウトでi番目にあるのは、頂点番号inds[i]

/**
 * メインメモリ上で保持するデータ
 * - xs:レイアウト
 * - ls:pos[左の頂点]
 * - rs:pos[右の頂点]
 * - root_pos: 根がレイアウトの何番目にあるか
 */
data_t* xs;
std::size_t* ls;
std::size_t* rs;
std::size_t root_pos;

/**
 * 頂点r以下の部分木に関するLayout
 * - block_height: ブロッキングの高さ
 */
template<std::size_t block_height>
void layout(const std::size_t r, std::size_t& index)
{
    const std::size_t height = lsb(r) + 1;
    const std::size_t offset = r - (1UL << (height - 1));
    const std::size_t uh     = (height - 1) % block_height + 1;  // rを含むブロックの高さ
    const std::size_t dh     = height - uh;
    for (std::size_t i = 1; i < (1UL << uh); i++) {
        inds[index++] = i * (1UL << dh) + offset;
    }
    if (dh != 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            layout<block_height>((1UL << (dh - 1)) * (i * 2 - 1) + offset, index);
        }
    }
}

template<std::size_t block_height>
void init()
{
    poss = new std::size_t[TN + 1];
    inds = new std::size_t[TN];
    xs   = new data_t[TN];
    ls   = new std::size_t[TN];
    rs   = new std::size_t[TN];

    std::size_t index = 0;
    layout<block_height>(R, index);
    for (std::size_t i = 0; i < TN; i++) {
        poss[inds[i]] = i;
    }
    root_pos = poss[R];
    for (std::size_t i = 0; i < TN; i++) {
        const std::size_t ind = inds[i];
        xs[i]                 = ind > N ? Inf : Xs[ind - 1];
        if ((ind & 1UL) == 0) {
            ls[i] = poss[left(ind)];
            rs[i] = poss[right(ind)];
        } else {
            ls[i] = TN;  // 無効な添字
            rs[i] = TN;  // 無効な添字
        }
    }
}

void fin()
{
    delete[] poss;
    delete[] inds;
    delete[] xs;
    delete[] ls;
    delete[] rs;
}

/**
 * クエリ応答
 * - 根から降りる
 */
inline data_t lower_bound(const data_t v)
{
    data_t ans = Inf;
    for (std::size_t pos = root_pos; pos != TN;) {
        const data_t x = xs[pos];
        if (x == v) { return v; }
        if (x < v) {
            pos = rs[pos];
        } else {
            ans = x;
            pos = ls[pos];
        }
    }
    return ans;
}

template<std::size_t block_height>
inline void test()
{
    data_t sum = 0;
    std::cout << "[Sol2] Blocking (Block Height: " << block_height << ")" << std::endl;
    SW.rap();
    for (std::size_t q = 0; q < Q; q++) {
        sum += lower_bound(Ys[q]);
    }
    const auto dur_ms = SW.rap<std::chrono::nanoseconds>();
    std::cout << "Query Total: " << dur_ms << " ns" << std::endl;
    std::cout << "Sum(for Debug): " << sum << std::endl;
    std::cout << std::endl;
}

}  // namespace blocking

namespace vEB {

constexpr std::size_t TN = ceil2(N + 1) - 1;  // x1~xNが含まれる完全二分探索木のサイズ
constexpr std::size_t R  = (TN + 1) / 2;      // 完全二分探索木の根

std::size_t* poss;  // 頂点番号iは、レイアウトでpos[i]番目に存在
std::size_t* inds;  // レイアウトでi番目にあるのは、頂点番号inds[i]

/**
 * メインメモリ上で保持するデータ
 * - xs:レイアウト
 * - ls:pos[左の頂点]
 * - rs:pos[右の頂点]
 * - root_pos: 根がレイアウトの何番目にあるか
 */
data_t* xs;
std::size_t* ls;
std::size_t* rs;
std::size_t root_pos;

/**
 * 頂点r以下の部分木に関するLayout
 */
void layout(const std::size_t r, std::size_t& index)
{
    if (r & 1UL) {
        inds[index++] = r;
        return;
    }
    const std::size_t height = lsb(r) + 1;
    const std::size_t offset = r - (1UL << (height - 1));
    const std::size_t uh     = height / 2;
    const std::size_t dh     = height - uh;
    std::size_t old_index    = index;
    layout(r / (1UL << dh), index);
    for (std::size_t i = old_index; i < index; i++) {
        inds[i] *= (1UL << dh);
    }
    if (dh > 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            layout((1UL << (dh - 1)) * (i * 2 - 1) + offset, index);
        }
    }
}

void init()
{
    poss = new std::size_t[TN + 1];
    inds = new std::size_t[TN];
    xs   = new data_t[TN];
    ls   = new std::size_t[TN];
    rs   = new std::size_t[TN];

    std::size_t index = 0;
    layout(R, index);
    for (std::size_t i = 0; i < TN; i++) {
        poss[inds[i]] = i;
    }
    root_pos = poss[R];
    for (std::size_t i = 0; i < TN; i++) {
        const std::size_t ind = inds[i];
        xs[i]                 = ind > N ? Inf : Xs[ind - 1];
        if ((ind & 1UL) == 0) {
            ls[i] = poss[left(ind)];
            rs[i] = poss[right(ind)];
        } else {
            ls[i] = TN;  // 無効な添字
            rs[i] = TN;  // 無効な添字
        }
    }
}

void fin()
{
    delete[] poss;
    delete[] inds;
    delete[] xs;
    delete[] ls;
    delete[] rs;
}

/**
 * クエリ応答
 * - 根から降りる
 */
inline data_t lower_bound(const data_t v)
{
    data_t ans = Inf;
    for (std::size_t pos = root_pos; pos != TN;) {
        const data_t x = xs[pos];
        if (x == v) { return v; }
        if (x < v) {
            pos = rs[pos];
        } else {
            ans = x;
            pos = ls[pos];
        }
    }
    return ans;
}

inline void test()
{
    data_t sum = 0;
    std::cout << "[Sol3] vEB Layout" << std::endl;
    SW.rap();
    for (std::size_t q = 0; q < Q; q++) {
        sum += lower_bound(Ys[q]);
    }
    const auto dur_ms = SW.rap<std::chrono::nanoseconds>();
    std::cout << "Query Total: " << dur_ms << " ns" << std::endl;
    std::cout << "Sum(for Debug): " << sum << std::endl;
    std::cout << std::endl;
}

}  // namespace vEB

int main()
{
    data_init();

    sorting::init();
    blocking::init<3>();
    blocking::init<4>();
    blocking::init<5>();
    blocking::init<6>();
    blocking::init<7>();
    vEB::init();

    sorting::test();
    blocking::test<3>();
    blocking::test<4>();
    blocking::test<5>();
    blocking::test<6>();
    blocking::test<7>();
    vEB::test();

    return 0;
}
