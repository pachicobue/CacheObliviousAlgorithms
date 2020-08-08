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
long long sum(const std::size_t B, const std::size_t M, safe_array<T> as)
{
    const std::size_t N = as.size();
    data_cache dcache(B, M);
    long long S = 0;
    for (std::size_t i = 0; i < N; i++) { S += dcache.disk_read<T>(reinterpret_cast<uintptr_t>(&as[i])); }
    long long actual = 0;
    for (std::size_t i = 0; i < N; i++) { actual += as[i]; }
    assert(S == actual);
    dcache.print_summary();
    return S;
}

int main()
{
    while (true) {
        std::cout << std::endl;
        std::cout << "# Summation Algorithm" << std::endl;
        std::cout << "Input: B M" << std::endl;
        std::cout << ">> ";
        std::size_t B, M;
        std::cin >> B >> M;
        std::cout << "Input: Type   (0: manual, 1:random)" << std::endl;
        std::cout << ">> ";
        int Type = 0;
        std::cin >> Type;
        std::cout << "Input: N" << std::endl;
        std::cout << ">> ";
        std::size_t N;
        std::cin >> N;
        if (Type == 0) {
            safe_array<T> as(B, N);
            std::cout << "Input: A[0] A[1] A[2] ... A[N-1]  (Be careful to avoid overflow!)" << std::endl;
            std::cout << ">> ";
            for (std::size_t i = 0; i < N; i++) {
                int a;
                std::cin >> a;
                as[i] = static_cast<T>(a);
            }
            const long long S = sum(B, M, as);
            std::cout << "Sum of A[] is " << S << std::endl;
        } else {
            safe_array<T> as(B, N);
            for (std::size_t i = 0; i < N; i++) { as[i] = rng.val<T>(-10, 10); }
            std::cout << "A: {";
            for (std::size_t i = 0; i < N; i++) { std::cout << +as[i] << " "; }
            std::cout << "}" << std::endl;
            const long long S = sum(B, M, as);
            std::cout << "Sum of A[] is " << S << std::endl;
        }
    }
    return 0;
}
