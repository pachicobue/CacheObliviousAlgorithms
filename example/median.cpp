/**
 * @file median.cpp
 * @brief 線形時間Selectアルゴリズムの実装
 */
#include <cassert>

#include "config.hpp"
#include "data_cache.hpp"
#include "gnuplot.hpp"
#include "rng_utility.hpp"

namespace {
static constexpr std::size_t N = (1 << 16);

data_t kth(const disk_vector<data_t>& as, const std::size_t K, data_cache& dcache)
{
    const std::size_t an = data_cache::disk_read_raw<std::size_t>(as.size_addr());
    if (an == 1) { return dcache.template disk_read<data_t>(as.addr(0)); }
    const std::size_t L = (an + 4) / 5;
    disk_vector<data_t> bs(L);
    for (std::size_t i = 0; i < an; i += 5) {
        std::vector<data_t> sub;  // 定数個の領域をメモリ上に持つことは許可することにする。
        for (std::size_t j = i; j < an and j < i + 5; j++) {
            sub.push_back(dcache.template disk_read<data_t>(as.addr(i)));
        }
        std::sort(sub.begin(), sub.end());
        const std::size_t n = sub.size();
        const data_t m      = sub[(n - 1) / 2];
        dcache.disk_write(bs.addr(i / 5), m);
    }
    const data_t X = kth(bs, (L - 1) / 2, dcache);
    std::size_t C = 0, D = 0, E = 0;
    for (std::size_t i = 0; i < an; i++) {
        const data_t a = dcache.template disk_read<data_t>(as.addr(i));
        (a < X ? C : a > X ? D : E)++;
    }
    if (C <= K and K < C + E) { return X; }
    if (C < K) {
        disk_vector<data_t> ds(D);
        for (std::size_t i = 0, d = 0; i < an; i++) {
            const data_t a = dcache.template disk_read<data_t>(as.addr(i));
            if (a > X) { dcache.disk_write(ds.addr(d++), a); }
        }
        return kth(ds, K - C - E, dcache);
    } else {
        disk_vector<data_t> cs(C);
        for (std::size_t i = 0, c = 0; i < an; i++) {
            const data_t a = dcache.template disk_read<data_t>(as.addr(i));
            if (a < X) { dcache.disk_write(cs.addr(c++), a); }
        }
        return kth(cs, K, dcache);
    }
}
}  // namespace

statistic_info Median(const std::size_t B, const std::size_t M)
{
    static rng_base<std::mt19937> rng(Seed);
    static std::vector<data_t> vs(N);
    static disk_vector<data_t> as(N);
    static bool flag                      = false;
    [[maybe_unused]] static data_t actual = 0;
    if (not flag) {
        flag = true;
        for (std::size_t i = 0; i < N; i++) {
            vs[i] = rng.val<data_t>(Min, Max);
            data_cache::disk_write_raw(as.addr(i), vs[i]);
        }
        std::sort(vs.begin(), vs.end());
        actual = vs[(N - 1) / 2];
    }
    data_cache dcache(B, M);
    [[maybe_unused]] const auto med = kth(as, (N - 1) / 2, dcache);
    assert(actual == med);
    return dcache.statistic();
}

int main()
{
    gp_stream gout;

    constexpr std::size_t MinB = 50;
    constexpr std::size_t M    = (1 << 20);
    std::cout << "# Median Complexity #" << std::endl;
    std::cout << ("#" + std::string(6, ' ') + "B") << " "
              << (std::string(7, ' ') + "M") << " "
              << (std::string(7, ' ') + "N") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << " "
              << (std::string(7, ' ') + "Q") << std::endl;

    gout << "set title \'Median Finding\'\n";
    gout << "set xlabel \'Cache Line Size [byte]\'\n";
    gout << "set ylabel \'Cache Miss [times]\'\n";
    {
        gout << "plot \'-\' u 1:6  title \'B-Q\' w lp\n";
        for (std::size_t B = MinB; B * B <= M; B++) {
            const auto [QR, QW] = Median(B, M);
            const auto Q        = QR + QW;
            std::cout << std::setw(8) << B << " "
                      << std::setw(8) << M << " "
                      << std::setw(8) << N << "   "
                      << std::setw(8) << QR << " "
                      << std::setw(8) << QW << " "
                      << std::setw(8) << Q << std::endl;
            gout << std::setw(8) << B << " "
                 << std::setw(8) << M << " "
                 << std::setw(8) << N << "   "
                 << std::setw(8) << QR << " "
                 << std::setw(8) << QW << " "
                 << std::setw(8) << Q << "\n";
        }
        gout << "e\n";
        gout.flush();
    }
    return 0;
}
