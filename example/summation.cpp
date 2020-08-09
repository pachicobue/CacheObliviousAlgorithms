/**
 * @file summation.cpp
 * @brief 線形走査の実装
 */
#include <cassert>

#include "data_cache.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"
/**
 * @brief 1点走査
 * @detail 
 * 例えば以下のような操作
 * キャッシュ計算量は Q <= ceil(N/B)+1 となる
 *
 * for i in 0..n-1:
 *     sum += A[i]
 */
using T = int8_t;
template<std::size_t B, std::size_t M>
void Summation(const std::size_t N)
{
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
    std::cout << N << " " << B << " " << M << "   " << QR << " " << QW << std::endl;
}

int main()
{
    std::cout << "# N B M : Q(Read) Q(Write)" << std::endl;
    Summation<1, 1000>(10000);
    Summation<10, 1000>(10000);
    Summation<100, 1000>(10000);
    Summation<1000, 1000>(10000);
    return 0;
}
