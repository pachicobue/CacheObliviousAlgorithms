/**
 * @file satatic_search.hpp
 * @brief 静的データの探索
 */
#include <cassert>
#include <numeric>

#include "bit_utility.hpp"
#include "config.hpp"
#include "data_cache.hpp"
#include "rng_utility.hpp"

namespace {

struct node_t
{
    data_t val{};
    std::size_t left = static_cast<std::size_t>(-1), right = static_cast<std::size_t>(-1);
};

inline std::size_t left(const std::size_t N)
{
    const std::size_t i = lsb(N) - 1;
    return N - (1UL << i);
}
inline std::size_t right(const std::size_t N)
{
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

template<typename F>
statistic_info static_search(const std::size_t B, const std::size_t M, const std::size_t N, const std::size_t T, F layout_func)
{
    rng_base<std::mt19937> rng(Seed);
    auto vs = rng.vec<data_t>(N, Min, Max);
    std::sort(vs.begin(), vs.end());
    std::size_t NN = 1;
    for (; NN < N; NN = NN * 2 + 1) {}
    const std::size_t R = (NN + 1) / 2;
    const auto layout   = layout_func(R);
    std::vector<std::size_t> poss(NN + 1);
    for (std::size_t i = 0; i < NN; i++) { poss[layout[i]] = i; }
    std::vector<node_t> nodes(NN);
    for (std::size_t i = 0; i < NN; i++) {
        nodes[i].val = layout[i] > N ? Max + 1 : vs[layout[i] - 1];
        if ((layout[i] & 1UL) == 0) { nodes[i].left = poss[left(layout[i])], nodes[i].right = poss[right(layout[i])]; }
    }
    auto lower_bound = [&](data_cache& dcache, const data_t& x) -> data_t {
        data_t ans = Max + 1;
        for (std::size_t ind = poss[R]; ind != static_cast<std::size_t>(-1);) {
            const node_t node = dcache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&nodes[ind]));
            if (node.val == x) { return x; }
            if (node.val < x) {
                ind = node.right;
            } else {
                ans = std::min(ans, node.val);
                ind = node.left;
            }
        }
        return ans;
    };
    vs.push_back(Max + 1);

    statistic_info info;
    for (std::size_t t = 0; t < T; t++) {
        data_cache dcache(B, M);
        const data_t qx     = rng.val<data_t>(Min, Max);
        const data_t actual = *std::lower_bound(vs.begin(), vs.end(), qx);
        const data_t ans    = lower_bound(dcache, qx);
        assert(actual == ans);
        dcache.flush();
        const auto [QR, QW] = dcache.statistic();
        info.disk_read_count += QR;
        info.disk_write_count += QW;
    }
    return info;
}
}  // namespace

statistic_info InOrderLayout(const std::size_t B, const std::size_t M, const std::size_t N, const std::size_t T)
{
    return static_search(B, M, N, T, in_order_layout);
}

statistic_info BlockLayout(const std::size_t B, const std::size_t M, const std::size_t H, std::size_t N, const std::size_t T)
{
    return static_search(B, M, N, T, [&](const std::size_t R) { return block_layout(R, H); });
}

statistic_info vEB_Layout(const std::size_t B, const std::size_t M, const std::size_t N, const std::size_t T)
{
    return static_search(B, M, N, T, vEB_layout);
}

int main()
{
    bool flags[]            = {true, true, true};
    constexpr std::size_t N = (1 << 16);
    constexpr std::size_t T = (1 << 8);
    constexpr std::size_t M = (1 << 16);
    std::cout << "# Static Search Complexity #" << std::endl;
    if (flags[0]) {
        std::cout << "# in-order layout" << std::endl;
        std::cout << ("#" + std::string(6, ' ') + "B") << " "
                  << (std::string(7, ' ') + "M") << " "
                  << (std::string(7, ' ') + "N") << " "
                  << (std::string(7, ' ') + "T") << " | "
                  << (std::string(6, ' ') + "QR") << " "
                  << (std::string(6, ' ') + "QW") << " "
                  << (std::string(7, ' ') + "Q") << std::endl;
        for (std::size_t B = 1; B * B <= M; B++) {
            const auto [QR, QW] = InOrderLayout(B, M, N, T);
            const auto Q        = QR + QW;
            std::cout << std::setw(8) << B << " "
                      << std::setw(8) << M << " "
                      << std::setw(8) << N << " "
                      << std::setw(8) << T << "   "
                      << std::setw(8) << QR << " "
                      << std::setw(8) << QW << " "
                      << std::setw(8) << Q << " "
                      << std::endl;
        }
        std::cout << std::endl;
    }
    if (flags[1]) {
        std::cout << "# block layout" << std::endl;
        std::cout << ("#" + std::string(6, ' ') + "B") << " "
                  << (std::string(7, ' ') + "M") << " "
                  << (std::string(7, ' ') + "H") << " "
                  << (std::string(7, ' ') + "N") << " "
                  << (std::string(7, ' ') + "T") << " | "
                  << (std::string(6, ' ') + "QR") << " "
                  << (std::string(6, ' ') + "QW") << " "
                  << (std::string(7, ' ') + "Q") << std::endl;
        std::size_t H  = 1;
        std::size_t SZ = sizeof(node_t);
        for (std::size_t B = 1; B * B <= M; B++) {
            if (SZ * 2 + 1 <= B) { H++, SZ = SZ * 2 + 1; }
            const auto [QR, QW] = BlockLayout(B, M, H, N, T);
            const auto Q        = QR + QW;
            std::cout << std::setw(8) << B << " "
                      << std::setw(8) << M << " "
                      << std::setw(8) << H << " "
                      << std::setw(8) << N << " "
                      << std::setw(8) << T << "   "
                      << std::setw(8) << QR << " "
                      << std::setw(8) << QW << " "
                      << std::setw(8) << Q << std::endl;
        }
        std::cout << std::endl;
    }
    if (flags[2]) {
        std::cout << "# vEB layout" << std::endl;
        std::cout << ("#" + std::string(6, ' ') + "B") << " "
                  << (std::string(7, ' ') + "M") << " "
                  << (std::string(7, ' ') + "N") << " "
                  << (std::string(7, ' ') + "T") << " | "
                  << (std::string(6, ' ') + "QR") << " "
                  << (std::string(6, ' ') + "QW") << " "
                  << (std::string(7, ' ') + "Q") << std::endl;
        for (std::size_t B = 1; B * B <= M; B++) {
            const auto [QR, QW] = vEB_Layout(B, M, N, T);
            const auto Q        = QR + QW;
            std::cout << std::setw(8) << B << " "
                      << std::setw(8) << M << " "
                      << std::setw(8) << N << " "
                      << std::setw(8) << T << "   "
                      << std::setw(8) << QR << " "
                      << std::setw(8) << QW << " "
                      << std::setw(8) << Q << " "
                      << std::endl;
        }
    }
    return 0;
}
