#include <iomanip>
#include <iostream>

#include "common/rng.hpp"
#include "sim_algorithm/b_tree.hpp"
#include "sim_algorithm/binary_search.hpp"
#include "sim_algorithm/block_search.hpp"
#include "sim_algorithm/vEB_search.hpp"
#include "simulator/simulator.hpp"

int main()
{
    constexpr std::size_t B = (1 << 9);
    constexpr std::size_t M = (1 << 18);
    constexpr std::size_t N = (1 << 24) + 64;
    constexpr std::size_t Q = (1 << 20);

    rng_base rng{Seed};
    const auto vs  = rng.vec<data_t>(N, Min, Max);
    const auto qxs = rng.vec<data_t>(Q, Min, Max);

    {
        std::cout << "[Sol1] Sorting" << std::endl;
        binary_search searcher{vs};
        std::cout << "Precalc end." << std::endl;
        sim::initialize(B, M);  // リセット
        for (std::size_t q = 0; q < Q; q++) {
            const data_t qx                 = qxs[q];
            [[maybe_unused]] const auto ans = searcher.lower_bound(qx);
        }
        const auto [R, W] = sim::cache_miss_count();
        assert(W == 0);
        const uint64_t QTotal = R + W;
        std::cout << "Cache Miss: " << QTotal << std::endl;
        std::cout << std::endl;
    }
    {
        for (std::size_t H = 3; H <= 7; H++) {
            std::cout << "[Sol2] Blocking (Block Height: " << H << ")" << std::endl;
            block_search searcher{vs, H};
            std::cout << "Precalc end." << std::endl;
            sim::initialize(B, M);  // リセット
            for (std::size_t q = 0; q < Q; q++) {
                const data_t qx                 = qxs[q];
                [[maybe_unused]] const auto ans = searcher.lower_bound(qx);
            }
            const auto [R, W] = sim::cache_miss_count();
            assert(W == 0);
            const uint64_t QTotal = R + W;
            std::cout << "Cache Miss: " << QTotal << std::endl;
            std::cout << std::endl;
        }
    }
    {
        std::cout << "[Sol3] vEB Layout" << std::endl;
        vEB_search searcher{vs};
        std::cout << "Precalc end." << std::endl;
        sim::initialize(B, M);  // リセット
        for (std::size_t q = 0; q < Q; q++) {
            const data_t qx                 = qxs[q];
            [[maybe_unused]] const auto ans = searcher.lower_bound(qx);
        }
        const auto [R, W] = sim::cache_miss_count();
        assert(W == 0);
        const uint64_t QTotal = R + W;
        std::cout << "Cache Miss: " << QTotal << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
