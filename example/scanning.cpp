/**
 * @file summation.cpp
 * @brief 線形走査の実装
 */
#include <cassert>

#include "config.hpp"
#include "data_cache.hpp"
#include "rng_utility.hpp"

statistic_info Scanning(const std::size_t B, const std::size_t M, const std::size_t N)
{
    rng_base<std::mt19937> rng(Seed);
    disk_vector<data_t> as(N);
    disk_vector<data_t> bs(N);
    data_cache dcache(B, M);
    for (std::size_t i = 0; i < N; i++) {
        data_cache::disk_write_raw(as.addr(i), rng.val<data_t>(Min, Max));
        data_cache::disk_write_raw(bs.addr(i), rng.val<data_t>(Min, Max));
    }
    disk_vector<data_t> cs(N);
    for (std::size_t i = 0; i < N; i++) {
        const data_t sum = dcache.template disk_read<data_t>(as.addr(i))
                           + dcache.template disk_read<data_t>(bs.addr(i));
        dcache.disk_write(cs.addr(i), sum);
    }
    for (std::size_t i = 0; i < N; i++) {
        const data_t a = data_cache::disk_read_raw<data_t>(as.addr(i));
        const data_t b = data_cache::disk_read_raw<data_t>(bs.addr(i));
        const data_t c = data_cache::disk_read_raw<data_t>(cs.addr(i));
        assert(c == a + b);
    }
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
    constexpr std::size_t N = (1 << 16);
    constexpr std::size_t M = (1 << 16);
    std::cout << "# Linear-Scanning Complexity #" << std::endl;
    std::cout << ("#" + std::string(6, ' ') + "B") << " "
              << (std::string(7, ' ') + "M") << " "
              << (std::string(7, ' ') + "N") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << " "
              << (std::string(7, ' ') + "Q") << std::endl;
    {
        for (std::size_t B = 1; B * B <= M; B++) {
            const auto [QR, QW] = Scanning(B, M, N);
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
