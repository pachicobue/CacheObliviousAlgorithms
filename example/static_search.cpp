/**
 * @file satatic_search.hpp
 * @brief 静的データの探索
 */
#include <numeric>

#include "bit_utility.hpp"
#include "data_cache.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"
namespace {
using T                 = int;
constexpr T min         = std::numeric_limits<T>::min() / 4;
constexpr T max         = std::numeric_limits<T>::max() / 4;
constexpr uint64_t seed = 20190810;

struct node_impl_t
{
    T val{};
    std::size_t left = static_cast<std::size_t>(-1), right = static_cast<std::size_t>(-1);
};
struct node_t
{
    node_impl_t node;
    std::byte reserved[ceil2(sizeof(node_impl_t)) - sizeof(node_impl_t)];
};
inline std::size_t left(const std::size_t N)
{
    assert((N & 1UL) == 0UL);
    const std::size_t i = lsb(N) - 1;
    return N - (1UL << i);
}
inline std::size_t right(const std::size_t N)
{
    assert((N & 1UL) == 0UL);
    const std::size_t i = lsb(N) - 1;
    return N + (1UL << i);
}

/**
 * @brief 完全に二分探索木のin-order順
 * @detail 普通の二分探索に相当
 */
std::vector<std::size_t> in_order_layout(const std::size_t R)
{
    std::vector<std::size_t> ans(2 * R - 1);
    std::iota(ans.begin(), ans.end(), 1);
    return ans;
}

/**
 * @brief 完全に二分探索木で、H段の部分木をBlockとして連続するように並べる(端数は根に来るようにする)
 * @detail B-木に相当
 */
std::vector<std::size_t> block_layout(const std::size_t R, const std::size_t H)
{
    const std::size_t height = lsb(R) + 1;
    const std::size_t offset = R - (1UL << (height - 1));
    const std::size_t uh     = (height - 1) % H + 1;
    const std::size_t dh     = height - uh;
    std::vector<std::size_t> ans;
    for (std::size_t i = 1; i < (1UL << uh); i++) { ans.push_back(i * (1UL << dh) + offset); }
    if (dh != 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            const auto sub = block_layout((1UL << (dh - 1)) * (i * 2 - 1) + offset, H);
            for (const std::size_t e : sub) { ans.push_back(e); }
        }
    }
    return ans;
}

/**
 * @brief vEB layoutで並べる
 * @details block_layoutを再帰的に行う感じ
 */
std::vector<std::size_t> vEB_layout(const std::size_t R)
{
    if (R & 1UL) { return std::vector<std::size_t>{R}; }
    const std::size_t height     = lsb(R) + 1;
    const std::size_t offset     = R - (1UL << (height - 1));
    const std::size_t uh         = height / 2;
    const std::size_t dh         = height - uh;
    std::vector<std::size_t> ans = vEB_layout(R / (1UL << dh));
    for (std::size_t& e : ans) { e *= (1UL << dh); }
    if (dh > 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            const auto sub = vEB_layout((1UL << (dh - 1)) * (i * 2 - 1) + offset);
            for (const std::size_t e : sub) { ans.push_back(e); }
        }
    }
    return ans;
}

template<std::size_t B, std::size_t M, typename F>
inline uint64_t static_search(const std::size_t N, const std::size_t Q, F layout_func)
{
    rng_base<std::mt19937> rng(seed);
    auto vs = rng.vec<T>(N, min, max);
    std::sort(vs.begin(), vs.end());
    std::size_t NN = 1;
    for (; NN < N; NN = NN * 2 + 1) {}
    const std::size_t R = (NN + 1) / 2;
    const auto layout   = layout_func(R);
    std::vector<std::size_t> poss(NN + 1);
    for (std::size_t i = 0; i < NN; i++) { poss[layout[i]] = i; }
    safe_array<node_t, B> nodes(NN);
    for (std::size_t i = 0; i < NN; i++) {
        nodes[i].node.val = layout[i] > N ? max + 1 : vs[layout[i] - 1];
        if ((layout[i] & 1UL) == 0) { nodes[i].node.left = poss[left(layout[i])], nodes[i].node.right = poss[right(layout[i])]; }
    }
    data_cache<B, M> dcache;
    auto lower_bound = [&](const T& x) -> T {
        T ans = max + 1;
        for (std::size_t ind = poss[R]; ind != static_cast<std::size_t>(-1);) {
            const node_t node = dcache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&nodes[ind]));
            if (node.node.val == x) { return x; }
            if (node.node.val < x) {
                ind = node.node.right;
            } else {
                ans = std::min(ans, node.node.val);
                ind = node.node.left;
            }
        }
        return ans;
    };
    vs.push_back(max + 1);
    uint64_t QR = 0;
    for (std::size_t q = 0; q < Q; q++) {
        const T qx     = rng.val<T>(min, max);
        const T actual = *std::lower_bound(vs.begin(), vs.end(), qx);
        const T ans    = lower_bound(qx);
        assert(actual == ans);
        dcache.flush();
        QR += dcache.statistic().disk_read_count;
        dcache.reset();
    }
    return QR;
}
}  // namespace

template<std::size_t B, std::size_t M>
inline void InOrderLayout(std::size_t N, const std::size_t Q)
{
    const uint64_t QR = static_search<B, M>(N, Q, in_order_layout);
    std::cout << std::setw(10) << B << " "
              << std::setw(10) << M << " "
              << std::setw(10) << sizeof(node_t) << " "
              << std::setw(10) << N << " "
              << std::setw(12) << Q << "   "
              << std::setw(10) << QR << std::endl;
}

template<std::size_t B, std::size_t M, std::size_t H>
inline void BlockLayout(std::size_t N, const std::size_t Q)
{
    const uint64_t QR = static_search<B, M>(N, Q, [&](const std::size_t R) { return block_layout(R, H); });
    std::cout << std::setw(10) << B << " "
              << std::setw(10) << M << " "
              << std::setw(10) << sizeof(node_t) << " "
              << std::setw(10) << H << " "
              << std::setw(10) << sizeof(node_t) * ((1UL << H) - 1) << " "
              << std::setw(10) << N << " "
              << std::setw(12) << Q << "   "
              << std::setw(10) << QR << std::endl;
}

template<std::size_t B, std::size_t M>
inline void vEB_Layout(std::size_t N, const std::size_t Q)
{
    const uint64_t QR = static_search<B, M>(N, Q, vEB_layout);
    std::cout << std::setw(10) << B << " "
              << std::setw(10) << M << " "
              << std::setw(10) << sizeof(node_t) << " "
              << std::setw(10) << N << " "
              << std::setw(12) << Q << "   "
              << std::setw(10) << QR << std::endl;
}

int main()
{
    bool flags[]            = {true, true, true};
    const std::size_t N     = (1 << 20);
    const std::size_t Q     = (1 << 0);
    constexpr std::size_t M = (1 << 30);
    if (flags[0]) {
        std::cout << "# InOrder Layout #" << std::endl;
        std::cout << ("#" + std::string(8, ' ') + "B") << " "
                  << (std::string(9, ' ') + "M") << " "
                  << (std::string(8, ' ') + "NS") << " "
                  << (std::string(9, ' ') + "N") << " "
                  << (std::string(11, ' ') + "Q") << " | "
                  << (std::string(8, ' ') + "QR") << std::endl;
        InOrderLayout<(1 << 6), M>(N, Q);
        InOrderLayout<(1 << 7), M>(N, Q);
        InOrderLayout<(1 << 8), M>(N, Q);
        InOrderLayout<(1 << 9), M>(N, Q);
        InOrderLayout<(1 << 10), M>(N, Q);
        InOrderLayout<(1 << 11), M>(N, Q);
        InOrderLayout<(1 << 12), M>(N, Q);
        InOrderLayout<(1 << 13), M>(N, Q);
        InOrderLayout<(1 << 14), M>(N, Q);
        InOrderLayout<(1 << 15), M>(N, Q);
        std::cout << std::endl;
    }
    if (flags[1]) {
        std::cout << "# Block Layout #" << std::endl;
        std::cout << ("#" + std::string(8, ' ') + "B") << " "
                  << (std::string(9, ' ') + "M") << " "
                  << (std::string(8, ' ') + "NS") << " "
                  << (std::string(9, ' ') + "H") << " "
                  << (std::string(8, ' ') + "BS") << " "
                  << (std::string(9, ' ') + "N") << " "
                  << (std::string(11, ' ') + "Q") << " | "
                  << (std::string(8, ' ') + "QR") << std::endl;
        BlockLayout<(1 << 6), M, 1>(N, Q);
        BlockLayout<(1 << 7), M, 1>(N, Q);
        BlockLayout<(1 << 8), M, 2>(N, Q);
        BlockLayout<(1 << 9), M, 3>(N, Q);
        BlockLayout<(1 << 10), M, 4>(N, Q);
        BlockLayout<(1 << 11), M, 5>(N, Q);
        BlockLayout<(1 << 12), M, 6>(N, Q);
        BlockLayout<(1 << 13), M, 7>(N, Q);
        BlockLayout<(1 << 14), M, 8>(N, Q);
        BlockLayout<(1 << 15), M, 9>(N, Q);
        std::cout << std::endl;
    }
    if (flags[2]) {
        std::cout << "# vEB Layout #" << std::endl;
        std::cout << ("#" + std::string(8, ' ') + "B") << " "
                  << (std::string(9, ' ') + "M") << " "
                  << (std::string(8, ' ') + "NS") << " "
                  << (std::string(9, ' ') + "N") << " "
                  << (std::string(11, ' ') + "Q") << " | "
                  << (std::string(8, ' ') + "QR") << std::endl;
        vEB_Layout<(1 << 6), M>(N, Q);
        vEB_Layout<(1 << 7), M>(N, Q);
        vEB_Layout<(1 << 8), M>(N, Q);
        vEB_Layout<(1 << 9), M>(N, Q);
        vEB_Layout<(1 << 10), M>(N, Q);
        vEB_Layout<(1 << 11), M>(N, Q);
        vEB_Layout<(1 << 12), M>(N, Q);
        vEB_Layout<(1 << 13), M>(N, Q);
        vEB_Layout<(1 << 14), M>(N, Q);
        vEB_Layout<(1 << 15), M>(N, Q);
        std::cout << std::endl;
    }
    return 0;
}
