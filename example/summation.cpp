/**
 * @file summation.cpp
 * @brief 線形走査の実装
 */
#include <cassert>

#include "data_cache.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"
namespace {
using T                 = int8_t;
constexpr uint64_t seed = 20190810;
}  // namespace

template<std::size_t B, std::size_t M>
void Summation(const std::size_t N)
{
    rng_base<std::mt19937> rng(seed);
    safe_array<T, B> as(N);
    for (std::size_t i = 0; i < N; i++) { as[i] = rng.val<T>(-100, 100); }
    data_cache<B, M> dcache;
    long long S = 0;
    for (std::size_t i = 0; i < N; i++) { S += dcache.template disk_read<T>(reinterpret_cast<uintptr_t>(&as[i])); }
    long long actual = 0;
    for (std::size_t i = 0; i < N; i++) { actual += as[i]; }
    assert(S == actual);
    const std::size_t QR = dcache.statistic().disk_read_count;
    const std::size_t QW = dcache.statistic().disk_write_count;
    std::cout << std::setw(8) << N << " "
              << std::setw(8) << B << " "
              << std::setw(8) << M << " | "
              << std::setw(8) << QR << " "
              << std::setw(8) << QW << std::endl;
}

int main()
{
    std::cout << ("#" + std::string(6, ' ') + "N") << " "
              << (std::string(7, ' ') + "B") << " "
              << (std::string(7, ' ') + "M") << " | "
              << (std::string(6, ' ') + "QR") << " "
              << (std::string(6, ' ') + "QW") << std::endl;
    {
        const std::size_t N     = 16384;
        constexpr std::size_t M = 8192;
        Summation<1, M>(N);
        Summation<2, M>(N);
        Summation<4, M>(N);
        Summation<8, M>(N);
        Summation<16, M>(N);
        Summation<32, M>(N);
        Summation<64, M>(N);
        std::cout << std::endl;
    }
    {
        const std::size_t N     = 32768;
        constexpr std::size_t M = 8192;
        Summation<1, M>(N);
        Summation<2, M>(N);
        Summation<4, M>(N);
        Summation<8, M>(N);
        Summation<16, M>(N);
        Summation<32, M>(N);
        Summation<64, M>(N);
        std::cout << std::endl;
    }
    {
        const std::size_t N     = 65536;
        constexpr std::size_t M = 8192;
        Summation<1, M>(N);
        Summation<2, M>(N);
        Summation<4, M>(N);
        Summation<8, M>(N);
        Summation<16, M>(N);
        Summation<32, M>(N);
        Summation<64, M>(N);
    }
    return 0;
}
