/**
 * @file b_tree.cpp
 * @brief B木を用いた二分探索の実装
 */
#include <cassert>
#include <limits>

#include "b_tree.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"
namespace {
using T                 = int;
constexpr T min         = std::numeric_limits<T>::min() / 4;
constexpr T max         = std::numeric_limits<T>::max() / 4;
constexpr uint64_t seed = 20190810;
}  // namespace

template<std::size_t K, std::size_t B, std::size_t M>
void BTree(const std::size_t N, const std::size_t Q)
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
    b_tree<T, K, B, M, false> btree(vs);  // キャッシュミス回数だけ欲しいのでキャッシングはOFF
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
              << std::setw(8) << btree.node_num() << " "
              << std::setw(8) << B << " "
              << std::setw(10) << M << "   "
              << std::setw(8) << QR << " "
              << std::setw(8) << QW << std::endl;
}

int main()
{
    std::cout << ("#" + std::string(6, ' ') + "N") << " "
              << (std::string(7, ' ') + "Q") << " "
              << (std::string(7, ' ') + "K") << " "
              << (std::string(6, ' ') + "NS") << " "
              << (std::string(6, ' ') + "NN") << " "
              << (std::string(7, ' ') + "B") << " "
              << (std::string(9, ' ') + "M") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << std::endl;
    {
        const std::size_t N     = (1 << 20);
        const std::size_t Q     = (1 << 0);
        constexpr std::size_t M = (1 << 30);
        BTree<2, (1 << 6), M>(N, Q);
        BTree<5, (1 << 7), M>(N, Q);
        BTree<10, (1 << 8), M>(N, Q);
        BTree<21, (1 << 9), M>(N, Q);
        BTree<42, (1 << 10), M>(N, Q);
        BTree<85, (1 << 11), M>(N, Q);
        BTree<170, (1 << 12), M>(N, Q);
        BTree<341, (1 << 13), M>(N, Q);
        BTree<682, (1 << 14), M>(N, Q);
        BTree<1365, (1 << 15), M>(N, Q);
    }
    return 0;
}
