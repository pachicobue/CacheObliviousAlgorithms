/**
 * @file satatic_search.hpp
 * @brief 静的データの探索
 */
#include <cassert>
#include <numeric>

#include "bit_utility.hpp"
#include "config.hpp"
#include "data_cache.hpp"
#include "gnuplot.hpp"
#include "rng_utility.hpp"

namespace {

bool flags[]             = {true, true, true};
constexpr std::size_t N  = (1 << 20);
constexpr std::size_t NN = ceil2(N + 1) - 1;
constexpr std::size_t R  = (NN + 1) / 2;
constexpr std::size_t T  = (1 << 12);

struct node_t
{
    data_t val{};
    std::size_t left = static_cast<std::size_t>(-1), right = static_cast<std::size_t>(-1);
};

constexpr std::size_t LayoutKind               = 3;
std::array<std::size_t, LayoutKind> StartIndss = {0, 0, 0};
std::array<std::size_t, LayoutKind> Hss        = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
std::array<disk_array<std::size_t, NN>, LayoutKind> Layoutss;
std::array<disk_array<node_t, NN>, LayoutKind> Nodess;

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

/**
 * @brief 完全に二分探索木のin-order順
 * @detail 普通の二分探索に相当
 */
void in_order_layout(const std::size_t r, std::size_t& /*index*/)
{
    for (std::size_t i = 0; i < 2 * r - 1; i++) {
        data_cache::disk_write_raw(Layoutss[0].addr(i), i + 1);
    }
}

/**
 * @brief 完全に二分探索木で、H段の部分木をBlockとして連続するように並べる(端数は根に来るようにする)
 * @detail B-木に相当
 */
void block_layout(const std::size_t r, const std::size_t h, std::size_t& index)
{
    const std::size_t height = lsb(r) + 1;
    const std::size_t offset = r - (1UL << (height - 1));
    const std::size_t uh     = (height - 1) % h + 1;
    const std::size_t dh     = height - uh;
    std::vector<std::size_t> ans;
    for (std::size_t i = 1; i < (1UL << uh); i++) {
        data_cache::disk_write_raw(Layoutss[1].addr(index++), i * (1UL << dh) + offset);
    }
    if (dh != 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            block_layout((1UL << (dh - 1)) * (i * 2 - 1) + offset, h, index);
        }
    }
}

/**
 * @brief vEB layoutで並べる
 * @details block_layoutを再帰的に行う感じ
 */
void vEB_layout(const std::size_t r, std::size_t& index)
{
    if (r & 1UL) {
        data_cache::disk_write_raw(Layoutss[2].addr(index++), r);
        return;
    }
    const std::size_t height = lsb(r) + 1;
    const std::size_t offset = r - (1UL << (height - 1));
    const std::size_t uh     = height / 2;
    const std::size_t dh     = height - uh;
    std::size_t old_index    = index;
    vEB_layout(r / (1UL << dh), index);
    for (std::size_t i = old_index; i < index; i++) {
        data_cache::disk_write_raw(Layoutss[2].addr(i), data_cache::disk_read_raw<std::size_t>(Layoutss[2].addr(i)) * (1UL << dh));
    }
    if (dh > 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            vEB_layout((1UL << (dh - 1)) * (i * 2 - 1) + offset, index);
        }
    }
}

std::vector<statistic_info> static_search(const std::size_t B, const std::size_t M, const std::size_t H, const std::size_t func_id)
{
    static rng_base<std::mt19937> rng(Seed);
    static std::vector<data_t> vss[] = {[&](std::vector<data_t>&& vs) {
                                            return std::sort(vs.begin(), vs.end()), vs.push_back(Max + 1), vs;
                                        }(rng.vec<data_t>(N, Min, Max)),
                                        [&](std::vector<data_t>&& vs) {
                                            return std::sort(vs.begin(), vs.end()), vs.push_back(Max + 1), vs;
                                        }(rng.vec<data_t>(N, Min, Max)),
                                        [&](std::vector<data_t>&& vs) {
                                            return std::sort(vs.begin(), vs.end()), vs.push_back(Max + 1), vs;
                                        }(rng.vec<data_t>(N, Min, Max))};
    if (Hss[func_id] != H) {
        Hss[func_id]      = H;
        std::size_t index = 0;
        if (func_id == 0) {
            in_order_layout(R, index);
        } else if (func_id == 1) {
            block_layout(R, H, index);
        } else if (func_id == 2) {
            vEB_layout(R, index);
        } else {
            assert(false);
        }
        std::vector<std::size_t> poss(NN + 1);
        const auto& layout = Layoutss[func_id];
        const auto& vs     = vss[func_id];
        for (std::size_t i = 0; i < NN; i++) {
            poss[data_cache::disk_read_raw<std::size_t>(layout.addr(i))] = i;
        }
        StartIndss[func_id] = poss[R];
        for (std::size_t i = 0; i < NN; i++) {
            const std::size_t ord = data_cache::disk_read_raw<std::size_t>(layout.addr(i));
            node_t node;
            node.val = ord > N ? Max + 1 : vs[ord - 1];
            if ((ord & 1UL) == 0) {
                node.left  = poss[left(ord)];
                node.right = poss[right(ord)];
            }
            data_cache::disk_write_raw(Nodess[func_id].addr(i), node);
        }
    }

    auto lower_bound = [&](data_cache& dcache, const data_t& x) -> data_t {
        data_t ans = Max + 1;
        for (std::size_t ind = StartIndss[func_id]; ind != static_cast<std::size_t>(-1);) {
            const node_t node = dcache.template disk_read<node_t>(Nodess[func_id].addr(ind));
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
    std::vector<statistic_info> infos;
    for (std::size_t t = 0; t < T; t++) {
        data_cache dcache(B, M);
        const data_t qx                   = t == 0 ? Min : t + 1 == T ? Max : rng.val<data_t>(Min, Max);
        [[maybe_unused]] const data_t ans = lower_bound(dcache, qx);
#ifndef NDEBUG
        [[maybe_unused]] const data_t actual = *std::lower_bound(vss[func_id].begin(), vss[func_id].end(), qx);
        assert(actual == ans);
#endif
        dcache.flush();
        infos.push_back(dcache.statistic());
    }
    return infos;
}
}  // namespace

std::vector<statistic_info> InOrderLayout(const std::size_t B, const std::size_t M)
{
    return static_search(B, M, 0, 0);
}

std::vector<statistic_info> BlockLayout(const std::size_t B, const std::size_t M, const std::size_t H)
{
    return static_search(B, M, H, 1);
}

std::vector<statistic_info> vEB_Layout(const std::size_t B, const std::size_t M)
{
    return static_search(B, M, 0, 2);
}

int main()
{
    gp_stream gout1;  // Total CacheMiss
    gp_stream gout2;  // Max CacheMiss
    std::cout << "# Static Search Complexity #" << std::endl;
    gout1 << "set size 4,4\n";
    gout1 << "set multiplot layout 2,2\n";
    gout1 << "set title \'Searching For Static Data\'\n";
    gout1 << "set xlabel \'Cache Line Size [byte]\'\n";
    gout1 << "set ylabel \'Cache Miss (Max) [times]\'\n";
    gout1 << "set logscale x\n";

    gout2 << "set size 4,4\n";
    gout2 << "set multiplot layout 2,2\n";
    gout2 << "set title \'Searching For Static Data\'\n";
    gout2 << "set xlabel \'Cache Line Size [byte]\'\n";
    gout2 << "set ylabel \'Cache Miss (Total) [times]\'\n";
    gout2 << "set logscale x\n";
    constexpr std::size_t MinB = 50;
    constexpr std::size_t M    = (1 << 24);
    if (flags[0]) {
        gout1 << "plot \'-\' u 1:2 title \'InOrder Layout(B-Q)\' w lp\n";
        gout2 << "plot \'-\' u 1:2 title \'InOrder Layout(B-Q)\' w lp\n";

        std::cout << "# in-order layout" << std::endl;
        std::cout << ("#" + std::string(6, ' ') + "B") << " "
                  << (std::string(7, ' ') + "M") << " "
                  << (std::string(7, ' ') + "N") << " "
                  << (std::string(7, ' ') + "T") << " | "
                  << (std::string(6, ' ') + "QR") << " "
                  << (std::string(6, ' ') + "QW") << " "
                  << (std::string(7, ' ') + "Q") << " "
                  << (std::string(2, ' ') + "Q(Max)") << std::endl;
        for (std::size_t B = MinB; B * B <= M; B++) {
            uint64_t QR = 0, QW = 0;
            uint64_t QMax = 0;
            for (const auto [R, W] : InOrderLayout(B, M)) {
                QR += R, QW += W;
                QMax = std::max(QMax, R + W);
            }
            const uint64_t Q = QR + QW;
            std::cout << std::setw(8) << B << " "
                      << std::setw(8) << M << " "
                      << std::setw(8) << N << " "
                      << std::setw(8) << T << "   "
                      << std::setw(8) << QR << " "
                      << std::setw(8) << QW << " "
                      << std::setw(8) << Q << " "
                      << std::setw(8) << QMax << std::endl;
            gout1 << std::setw(8) << B << " "
                  << std::setw(8) << Q << "\n";
            gout2 << std::setw(8) << B << " "
                  << std::setw(8) << QMax << "\n";
        }
        gout1 << "e\n";
        gout1.flush();
        gout2 << "e\n";
        gout2.flush();
        std::cout << std::endl;
    }
    if (flags[1]) {
        gout1 << "plot \'-\' u 1:2 title \'Block Layout(B-Q)\' w lp\n";
        gout2 << "plot \'-\' u 1:2 title \'Block Layout(B-Q)\' w lp\n";

        std::cout << "# block layout" << std::endl;
        std::cout << ("#" + std::string(6, ' ') + "B") << " "
                  << (std::string(7, ' ') + "M") << " "
                  << (std::string(7, ' ') + "H") << " "
                  << (std::string(7, ' ') + "N") << " "
                  << (std::string(7, ' ') + "T") << " | "
                  << (std::string(6, ' ') + "QR") << " "
                  << (std::string(6, ' ') + "QW") << " "
                  << (std::string(7, ' ') + "Q") << " "
                  << (std::string(2, ' ') + "Q(Max)") << std::endl;
        std::size_t H  = 1;
        std::size_t SZ = sizeof(node_t);
        for (std::size_t B = MinB; B * B <= M; B++) {
            if (SZ * 2 + 1 <= B) { H++, SZ = SZ * 2 + 1; }
            uint64_t QR = 0, QW = 0;
            uint64_t QMax = 0;
            for (const auto [R, W] : BlockLayout(B, M, H)) {
                QR += R, QW += W;
                QMax = std::max(QMax, R + W);
            }
            const uint64_t Q = QR + QW;
            std::cout << std::setw(8) << B << " "
                      << std::setw(8) << M << " "
                      << std::setw(8) << H << " "
                      << std::setw(8) << N << " "
                      << std::setw(8) << T << "   "
                      << std::setw(8) << QR << " "
                      << std::setw(8) << QW << " "
                      << std::setw(8) << Q << " "
                      << std::setw(8) << QMax << std::endl;
            gout1 << std::setw(8) << B << " "
                  << std::setw(8) << Q << "\n";
            gout2 << std::setw(8) << B << " "
                  << std::setw(8) << QMax << "\n";
        }
        gout1 << "e\n";
        gout1.flush();
        gout2 << "e\n";
        gout2.flush();
        std::cout << std::endl;
    }
    if (flags[2]) {
        gout1 << "plot \'-\' u 1:2 title \'vEB Layout(B-Q)\' w lp\n";
        gout2 << "plot \'-\' u 1:2 title \'vEB Layout(B-Q)\' w lp\n";

        std::cout << "# vEB layout" << std::endl;
        std::cout << ("#" + std::string(6, ' ') + "B") << " "
                  << (std::string(7, ' ') + "M") << " "
                  << (std::string(7, ' ') + "N") << " "
                  << (std::string(7, ' ') + "T") << " | "
                  << (std::string(6, ' ') + "QR") << " "
                  << (std::string(6, ' ') + "QW") << " "
                  << (std::string(7, ' ') + "Q") << " "
                  << (std::string(2, ' ') + "Q(Max)") << std::endl;
        for (std::size_t B = MinB; B * B <= M; B++) {
            uint64_t QR = 0, QW = 0;
            uint64_t QMax = 0;
            for (const auto [R, W] : vEB_Layout(B, M)) {
                QR += R, QW += W;
                QMax = std::max(QMax, R + W);
            }
            const uint64_t Q = QR + QW;
            std::cout << std::setw(8) << B << " "
                      << std::setw(8) << M << " "
                      << std::setw(8) << N << " "
                      << std::setw(8) << T << "   "
                      << std::setw(8) << QR << " "
                      << std::setw(8) << QW << " "
                      << std::setw(8) << Q << " "
                      << std::setw(8) << QMax << std::endl;
            gout1 << std::setw(8) << B << " "
                  << std::setw(8) << Q << "\n";
            gout2 << std::setw(8) << B << " "
                  << std::setw(8) << QMax << "\n";
        }
        gout1 << "e\n";
        gout1.flush();
        gout2 << "e\n";
        gout2.flush();
    }
    return 0;
}
