/**
 * @file summation.cpp
 * @brief 線形走査の実装
 */
#include <cassert>

#include "config.hpp"
#include "data_cache.hpp"
#include "gnuplot.hpp"
#include "rng_utility.hpp"

namespace {
constexpr std::size_t N = (1 << 16);
}

statistic_info Scanning(const std::size_t B, const std::size_t M)
{
    static rng_base<std::mt19937> rng(Seed);
    static bool flag = false;
    static disk_vector<data_t> as(N);
    static disk_vector<data_t> bs(N);
    static disk_vector<data_t> cs(N);
    if (not flag) {
        flag = true;
        for (std::size_t i = 0; i < N; i++) {
            data_cache::disk_write_raw(as.addr(i), rng.val<data_t>(Min, Max));
            data_cache::disk_write_raw(bs.addr(i), rng.val<data_t>(Min, Max));
        }
    }
    data_cache dcache(B, M);
    for (std::size_t i = 0; i < N; i++) {
        const data_t sum = dcache.template disk_read<data_t>(as.addr(i))
                           + dcache.template disk_read<data_t>(bs.addr(i));
        dcache.disk_write(cs.addr(i), sum);
    }
#ifndef NDEBUG
    for (std::size_t i = 0; i < N; i++) {
        assert(data_cache::disk_read_raw<data_t>(cs.addr(i))
               == data_cache::disk_read_raw<data_t>(as.addr(i))
                      + data_cache::disk_read_raw<data_t>(bs.addr(i)));
    }
#endif
    return dcache.statistic();
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
    gp_stream gout;

    constexpr std::size_t MinB = 50;
    constexpr std::size_t M    = (1 << 20);
    std::cout << "# Linear-Scanning Complexity #" << std::endl;
    std::cout << ("#" + std::string(6, ' ') + "B") << " "
              << (std::string(7, ' ') + "M") << " "
              << (std::string(7, ' ') + "N") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << " "
              << (std::string(7, ' ') + "Q") << std::endl;

    gout << "set title \'Linear Scanning\'\n";
    gout << "set xlabel \'Cache Line Size [byte]\'\n";
    gout << "set ylabel \'Cache Miss [times]\'\n";
    {
        gout << "plot \'-\' u 1:6  title \'B-Q\' w lp\n";
        for (std::size_t B = MinB; B * B <= M; B++) {
            const auto [QR, QW] = Scanning(B, M);
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
