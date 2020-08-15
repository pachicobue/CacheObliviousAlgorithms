/**
 * @file median.cpp
 * @brief 線形時間Selectアルゴリズムの実装
 */
#include <cassert>

#include "data_cache.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"

namespace {
using T                 = int;
constexpr T min         = std::numeric_limits<T>::min();
constexpr T max         = std::numeric_limits<T>::max();
constexpr uint64_t seed = 20190810;
template<std::size_t B, std::size_t M>
T kth(const safe_array<T, B>& as, const std::size_t K, data_cache<B, M>& dcache)
{
    const std::size_t N = as.size();
    if (N == 1) { return dcache.template disk_read<T>(reinterpret_cast<uintptr_t>(&as[0])); }
    const std::size_t L = (N + 4) / 5;
    safe_array<T, B> bs(L);
    for (std::size_t i = 0; i < N; i += 5) {
        std::vector<T> sub;  // 定数個の領域をメモリ上に持つことは許可することにする。
        for (std::size_t j = i; j < N and j < i + 5; j++) { sub.push_back(dcache.template disk_read<T>(reinterpret_cast<uintptr_t>(&as[j]))); }
        std::sort(sub.begin(), sub.end());
        const std::size_t n = sub.size();
        const T m           = sub[(n - 1) / 2];
        dcache.disk_write(reinterpret_cast<uintptr_t>(&bs[i / 5]), m);
    }
    const T X     = kth(bs, (L - 1) / 2, dcache);
    std::size_t C = 0, D = 0, E = 0;
    for (std::size_t i = 0; i < N; i++) {
        const T a = dcache.template disk_read<T>(reinterpret_cast<uintptr_t>(&as[i]));
        (a < X ? C : a > X ? D : E)++;
    }
    if (C <= K and K < C + E) { return X; }
    if (C < K) {
        safe_array<T, B> ds(D);
        for (std::size_t i = 0, d = 0; i < N; i++) {
            const T a = dcache.template disk_read<T>(reinterpret_cast<uintptr_t>(&as[i]));
            if (a > X) { dcache.disk_write(reinterpret_cast<uintptr_t>(&ds[d++]), a); }
        }
        return kth(ds, K - C - E, dcache);
    } else {
        safe_array<T, B> cs(C);
        for (std::size_t i = 0, c = 0; i < N; i++) {
            const T a = dcache.template disk_read<T>(reinterpret_cast<uintptr_t>(&as[i]));
            if (a < X) { dcache.disk_write(reinterpret_cast<uintptr_t>(&cs[c++]), a); }
        }
        return kth(cs, K, dcache);
    }
}
}  // namespace

template<std::size_t B, std::size_t M>
void Median(const std::size_t N)
{
    rng_base<std::mt19937> rng(seed);
    safe_array<T, B> as(N);
    for (std::size_t i = 0; i < N; i++) { as[i] = rng.val<T>(min, max); }
    data_cache<B, M> dcache;
    const auto med = kth(as, (N - 1) / 2, dcache);
    std::vector<T> vs;
    for (std::size_t i = 0; i < N; i++) { vs.push_back(as[i]); }
    std::sort(vs.begin(), vs.end());
    const auto actual = vs[(N - 1) / 2];
    assert(actual == med);
    const std::size_t QR = dcache.statistic().disk_read_count;
    const std::size_t QW = dcache.statistic().disk_write_count;
    std::cout << std::setw(8) << N << " "
              << std::setw(8) << B << " "
              << std::setw(8) << M << "   "
              << std::setw(8) << QR << " "
              << std::setw(8) << QW << std::endl;
}

int main()
{
    std::cout << ("#" + std::string(6, ' ') + "N") << " "
              << (std::string(7, ' ') + "B") << " "
              << (std::string(7, ' ') + "M") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << std::endl;
    {
        const std::size_t N     = (1 << 18);
        constexpr std::size_t M = (1 << 18);
        Median<(1 << 0), M>(N);
        Median<(1 << 1), M>(N);
        Median<(1 << 2), M>(N);
        Median<(1 << 3), M>(N);
        Median<(1 << 4), M>(N);
        Median<(1 << 5), M>(N);
        Median<(1 << 6), M>(N);
        Median<(1 << 7), M>(N);
        Median<(1 << 8), M>(N);
    }
    return 0;
}
