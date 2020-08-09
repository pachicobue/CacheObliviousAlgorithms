/**
 * @file binary_search.cpp
 * @brief 二分探索の実装
 */
#include <cassert>

#include "b_tree.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"
namespace {
using T                 = int8_t;
constexpr T min         = -100;
constexpr T max         = 100;
constexpr uint64_t seed = 20190810;
}  // namespace

template<std::size_t K, std::size_t B, std::size_t M>
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
    b_tree<T, K, B, M> btree(vs);
    std::sort(vs.begin(), vs.end());
    for (std::size_t q = 0; q < Q; q++) {
        const T qx            = rng.val<T>(min, max);
        const auto it         = std::lower_bound(vs.begin(), vs.end(), qx);
        const auto [found, x] = btree.lower_bound(qx);
        if (not found) {
            assert(it == vs.end());
        } else {
            assert(*it == x);
        }
    }
    const std::size_t QR = btree.statistic().disk_read_count;
    const std::size_t QW = btree.statistic().disk_write_count;
    std::cout << std::setw(8) << N << " "
              << std::setw(8) << Q << " "
              << std::setw(8) << K << " "
              << std::setw(8) << btree.NodeSize << " "
              << std::setw(8) << B << " "
              << std::setw(8) << M << " | "
              << std::setw(8) << QR << " "
              << std::setw(8) << QW << std::endl;
}

int main()
{
    std::cout << ("#" + std::string(6, ' ') + "N") << " "
              << (std::string(7, ' ') + "Q") << " "
              << (std::string(7, ' ') + "K") << " "
              << (std::string(6, ' ') + "NS") << " "
              << (std::string(7, ' ') + "B") << " "
              << (std::string(7, ' ') + "M") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << std::endl;
    {
        constexpr std::size_t B = 64;
        constexpr std::size_t M = 8192;
        const std::size_t N     = 128;
        const std::size_t Q     = 16;
        BinSearch<1, B, M>(N, Q);
        BinSearch<2, B, M>(N, Q);
        BinSearch<4, B, M>(N, Q);
        BinSearch<8, B, M>(N, Q);
        BinSearch<16, B, M>(N, Q);
        BinSearch<32, B, M>(N, Q);
        BinSearch<64, B, M>(N, Q);
        std::cout << std::endl;
    }
    {
        constexpr std::size_t M = 8192;
        const std::size_t N     = 128;
        const std::size_t Q     = 16;
        BinSearch<1, 16, M>(N, Q);
        BinSearch<2, 32, M>(N, Q);
        BinSearch<4, 64, M>(N, Q);
        std::cout << std::endl;
    }
    return 0;
}
