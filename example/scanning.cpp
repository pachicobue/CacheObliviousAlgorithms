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
    std::vector<data_t> as(N);
    std::vector<data_t> bs(N);
    for (std::size_t i = 0; i < N; i++) {
        as[i] = rng.val<data_t>(Min, Max);
        bs[i] = rng.val<data_t>(Min, Max);
    }
    std::vector<data_t> cs(N);
    data_cache dcache(B, M);
    for (std::size_t i = 0; i < N; i++) {
        const data_t sum = dcache.template disk_read<data_t>(reinterpret_cast<uintptr_t>(&as[i]))
                           + dcache.template disk_read<data_t>(reinterpret_cast<uintptr_t>(&bs[i]));
        dcache.disk_write(reinterpret_cast<uintptr_t>(&cs[i]), sum);
    }
    for (std::size_t i = 0; i < N; i++) { assert(cs[i] == as[i] + bs[i]); }
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
