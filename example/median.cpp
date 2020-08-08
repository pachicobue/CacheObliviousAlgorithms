/**
 * @file median.cpp
 * @brief 線形時間Selectアルゴリズムの実装
 */
#include <cassert>

#include "data_cache.hpp"
#include "rng_utility.hpp"
#include "safe_array.hpp"

using T = int8_t;
T kth(const safe_array<T>& as, const std::size_t K, data_cache& dcache)  // 本当はarray全体を取らず、アドレスとサイズで受け取るほうが正しい
{
    const std::size_t N = as.size();
    const std::size_t B = dcache.page_size();
    if (N == 1) { return dcache.disk_read<T>(reinterpret_cast<uintptr_t>(&as[0])); }
    const std::size_t M = (N + 4) / 5;
    safe_array<T> bs(B, M);
    for (std::size_t i = 0; i < N; i += 5) {
        std::vector<T> sub;  // 定数個の領域をメモリ上に持つことは許可することにする。
        for (std::size_t j = i; j < N and j < i + 5; j++) { sub.push_back(dcache.disk_read<T>(reinterpret_cast<uintptr_t>(&as[j]))); }
        std::sort(sub.begin(), sub.end());
        const std::size_t n = sub.size();
        const T m           = sub[(n - 1) / 2];
        dcache.disk_write(reinterpret_cast<uintptr_t>(&bs[i / 5]), m);
    }
    const T X     = kth(bs, (M - 1) / 2, dcache);
    std::size_t C = 0, D = 0, E = 0;
    for (std::size_t i = 0; i < N; i++) {
        const T a = dcache.disk_read<T>(reinterpret_cast<uintptr_t>(&as[i]));
        (a < X ? C : a > X ? D : E)++;
    }
    if (C <= K and K < C + E) { return X; }
    if (C < K) {
        safe_array<T> ds(B, D);
        for (std::size_t i = 0, d = 0; i < N; i++) {
            const T a = dcache.disk_read<T>(reinterpret_cast<uintptr_t>(&as[i]));
            if (a > X) { dcache.disk_write(reinterpret_cast<uintptr_t>(&ds[d++]), a); }
        }
        return kth(ds, K - C - E, dcache);
    } else {
        safe_array<T> cs(B, C);
        for (std::size_t i = 0, c = 0; i < N; i++) {
            const T a = dcache.disk_read<T>(reinterpret_cast<uintptr_t>(&as[i]));
            if (a < X) { dcache.disk_write(reinterpret_cast<uintptr_t>(&cs[c++]), a); }
        }
        return kth(cs, K, dcache);
    }
}

T median(const std::size_t B, const std::size_t M, const std::size_t N, const safe_array<T>& as)
{
    data_cache dcache(B, M);
    const auto med = kth(as, (N - 1) / 2, dcache);
    std::vector<T> vs;
    for (std::size_t i = 0; i < N; i++) { vs.push_back(as[i]); }
    std::sort(vs.begin(), vs.end());
    const auto actual = vs[(N - 1) / 2];
    assert(actual == med);
    dcache.print_summary();
    return med;
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
        std::cout << "Input: Type   (0: manual test, 1:random test)" << std::endl;
        std::cout << ">> ";
        int Type = 0;
        std::cin >> Type;
        std::cout << "Input: N" << std::endl;
        std::cout << ">> ";
        std::size_t N;
        std::cin >> N;
        if (Type == 0) {
            safe_array<T> as(B, N);
            std::cout << "Input: A[0] A[1] A[2] ... A[N-1]" << std::endl;
            std::cout << ">> ";
            for (std::size_t i = 0; i < N; i++) {
                int a;
                std::cin >> a;
                as[i] = static_cast<T>(a);
            }
            const auto med = median(B, M, N, as);
            std::cout << "Median of A[] is " << +med << std::endl;
        } else {
            safe_array<T> as(B, N);
            for (std::size_t i = 0; i < N; i++) { as[i] = rng.val<T>(-10, 10); }
            median(B, M, N, as);
        }
    }
    return 0;
}
