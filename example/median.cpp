/**
 * @file median.cpp
 * @brief 線形時間Selectアルゴリズムの実装
 */
#include <cassert>

#include "config.hpp"
#include "data_cache.hpp"
#include "rng_utility.hpp"

namespace {

data_t kth(const std::vector<data_t>& as, const std::size_t K, data_cache& dcache)
{
    const std::size_t N = as.size();
    if (N == 1) { return dcache.template disk_read<data_t>(reinterpret_cast<uintptr_t>(&as[0])); }
    const std::size_t L = (N + 4) / 5;
    std::vector<data_t> bs(L);
    for (std::size_t i = 0; i < N; i += 5) {
        std::vector<data_t> sub;  // 定数個の領域をメモリ上に持つことは許可することにする。
        for (std::size_t j = i; j < N and j < i + 5; j++) { sub.push_back(dcache.template disk_read<data_t>(reinterpret_cast<uintptr_t>(&as[j]))); }
        std::sort(sub.begin(), sub.end());
        const std::size_t n = sub.size();
        const data_t m      = sub[(n - 1) / 2];
        dcache.disk_write(reinterpret_cast<uintptr_t>(&bs[i / 5]), m);
    }
    const data_t X = kth(bs, (L - 1) / 2, dcache);
    std::size_t C = 0, D = 0, E = 0;
    for (std::size_t i = 0; i < N; i++) {
        const data_t a = dcache.template disk_read<data_t>(reinterpret_cast<uintptr_t>(&as[i]));
        (a < X ? C : a > X ? D : E)++;
    }
    if (C <= K and K < C + E) { return X; }
    if (C < K) {
        std::vector<data_t> ds(D);
        for (std::size_t i = 0, d = 0; i < N; i++) {
            const data_t a = dcache.template disk_read<data_t>(reinterpret_cast<uintptr_t>(&as[i]));
            if (a > X) { dcache.disk_write(reinterpret_cast<uintptr_t>(&ds[d++]), a); }
        }
        return kth(ds, K - C - E, dcache);
    } else {
        std::vector<data_t> cs(C);
        for (std::size_t i = 0, c = 0; i < N; i++) {
            const data_t a = dcache.template disk_read<data_t>(reinterpret_cast<uintptr_t>(&as[i]));
            if (a < X) { dcache.disk_write(reinterpret_cast<uintptr_t>(&cs[c++]), a); }
        }
        return kth(cs, K, dcache);
    }
}
}  // namespace

statistic_info Median(const std::size_t B, const std::size_t M, const std::size_t N)
{
    rng_base<std::mt19937> rng(Seed);
    std::vector<data_t> as(N);
    for (std::size_t i = 0; i < N; i++) { as[i] = rng.val<data_t>(Min, Max); }
    data_cache dcache(B, M);
    const auto med = kth(as, (N - 1) / 2, dcache);
    std::vector<data_t> vs;
    for (std::size_t i = 0; i < N; i++) { vs.push_back(as[i]); }
    std::sort(vs.begin(), vs.end());
    const auto actual = vs[(N - 1) / 2];
    assert(actual == med);
    return dcache.statistic();
}

int main()
{
    constexpr std::size_t N = (1 << 16);
    constexpr std::size_t M = (1 << 16);
    std::cout << "# Median Complexity #" << std::endl;
    std::cout << ("#" + std::string(6, ' ') + "B") << " "
              << (std::string(7, ' ') + "M") << " "
              << (std::string(7, ' ') + "N") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << " "
              << (std::string(7, ' ') + "Q") << std::endl;
    {
        for (std::size_t B = 1; B * B <= M; B++) {
            const auto [QR, QW] = Median(B, M, N);
            const auto Q        = QR + QW;
            std::cout << std::setw(8) << B << " "
                      << std::setw(8) << M << " "
                      << std::setw(8) << N << "   "
                      << std::setw(8) << QR << " "
                      << std::setw(8) << QW << " "
                      << std::setw(8) << Q << std::endl;
        }
    }
    return 0;
}
