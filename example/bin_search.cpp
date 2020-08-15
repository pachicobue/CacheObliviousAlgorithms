/**
 * @file bin_search.cpp
 * @brief 二分探索の実装
 */
#include <cassert>
#include <limits>
#include <vector>

#include "data_cache.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"
namespace {
using T                 = int;
constexpr T min         = std::numeric_limits<T>::min();
constexpr T max         = std::numeric_limits<T>::max();
constexpr uint64_t seed = 20190810;
template<std::size_t B, std::size_t M>
std::pair<bool, T> lower_bound(const safe_array<T, B>& vs, const T& x, data_cache<B, M>& dcache)
{
    const std::size_t N = vs.size();
    int inf = -1, sup = N;
    while (sup - inf > 1) {
        const int mid = (inf + sup) / 2;
        const T v     = dcache.template disk_read<T>(reinterpret_cast<uintptr_t>(&vs[mid]));
        if (v == x) { return std::make_pair(true, x); }
        (v < x ? inf : sup) = mid;
    }
    if (sup == (int)N) {
        return std::make_pair(false, x);
    } else {
        return std::make_pair(true, vs[sup]);
    }
}

}  // namespace

template<std::size_t B, std::size_t M>
void BinSearch(const std::size_t N, const std::size_t Q)
{
    rng_base<std::mt19937> rng(seed);
    std::vector<T> vs(N);
    {
        std::set<T> used;
        for (std::size_t i = 0; i < N; i++) {
            T v;
            do {
                v = rng.val<T>(min, max);
            } while (used.count(v));
            used.insert(v);
            vs[i] = v;
        }
    }
    data_cache<B, M> dcache;
    std::sort(vs.begin(), vs.end());
    safe_array<T, B> svs(N);
    for (std::size_t i = 0; i < N; i++) { svs[i] = vs[i]; }
    for (std::size_t q = 0; q < Q; q++) {
        const T qx            = rng.val<T>(min, max);
        const auto it         = std::lower_bound(vs.begin(), vs.end(), qx);
        const auto [found, x] = lower_bound(svs, qx, dcache);
        if (not found) {
            assert(it == vs.end());
        } else {
            assert(*it == x);
        }
    }
    const std::size_t QR = dcache.statistic().disk_read_count;
    const std::size_t QW = dcache.statistic().disk_write_count;
    std::cout << std::setw(8) << N << " "
              << std::setw(8) << Q << " "
              << std::setw(8) << B << " "
              << std::setw(10) << M << "   "
              << std::setw(8) << QR << " "
              << std::setw(8) << QW << std::endl;
}

int main()
{
    std::cout << ("#" + std::string(6, ' ') + "N") << " "
              << (std::string(7, ' ') + "Q") << " "
              << (std::string(7, ' ') + "B") << " "
              << (std::string(9, ' ') + "M") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << std::endl;
    {
        const std::size_t N     = (1 << 20);
        const std::size_t Q     = (1 << 0);
        constexpr std::size_t M = (1 << 30);
        BinSearch<(1 << 6), M>(N, Q);
        BinSearch<(1 << 7), M>(N, Q);
        BinSearch<(1 << 8), M>(N, Q);
        BinSearch<(1 << 9), M>(N, Q);
        BinSearch<(1 << 10), M>(N, Q);
        BinSearch<(1 << 11), M>(N, Q);
        BinSearch<(1 << 12), M>(N, Q);
        BinSearch<(1 << 13), M>(N, Q);
        BinSearch<(1 << 14), M>(N, Q);
        BinSearch<(1 << 15), M>(N, Q);
    }
    return 0;
}
