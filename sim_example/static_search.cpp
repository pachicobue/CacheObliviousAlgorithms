#include <iomanip>
#include <iostream>

#include "common/gnuplot.hpp"
#include "common/rng.hpp"
#include "sim_algorithm/b_tree.hpp"
#include "sim_algorithm/binary_search.hpp"
#include "sim_algorithm/block_search.hpp"
#include "sim_algorithm/vEB_search.hpp"
#include "simulator/simulator.hpp"

int main()
{
    constexpr std::size_t MinB = (1 << 7);
    constexpr std::size_t MaxB = (1 << 15);
    constexpr std::size_t M    = (1 << 30);
    constexpr std::size_t N    = (1 << 16);
    constexpr std::size_t T    = (1 << 6);

    rng_base rng{Seed};
    const auto vs = rng.vec<data_t>(N, Min, Max);
    const std::set<data_t> st(vs.begin(), vs.end());
    std::vector<data_t> qxs(T);
    for (std::size_t t = 0; t < T; t++) {
        data_t qx = rng.val<data_t>(Min, Max);
        while (st.count(qx)) {
            qx = rng.val<data_t>(Min, Max);
        }
        qxs[t] = qx;
    }

    gp_stream gout;  // Total CacheMiss
    gout << "set size 4,4" << std::endl;
    gout << "set multiplot layout 2,2" << std::endl;
    gout << "set title \'Searching For Static Data\'" << std::endl;
    gout << "set xlabel \'Cache Line Size [byte]\'" << std::endl;
    gout << "set ylabel \'Cache Miss (Max) [times]\'" << std::endl;
    gout << "set logscale x" << std::endl;

    {
        binary_search searcher{vs};
        gout << "plot \'-\' u 1:2 title \'Binary Search (B-Q)\' w lp" << std::endl;
        std::cout << "# binary search" << std::endl;
        for (std::size_t B = MinB; B <= MaxB; B++) {
            uint64_t QMax = 0;
            for (std::size_t t = 0; t < T; t++) {
                sim::initialize(B, M);  // リセット
                const data_t qx                 = qxs[t];
                [[maybe_unused]] const auto ans = searcher.lower_bound(qx);
                const auto [R, W]               = sim::cache_miss_count();
                QMax                            = std::max(QMax, R + W);
            }
            gout << std::setw(8) << B << " "
                 << std::setw(8) << QMax << std::endl;
        }
        gout << "e" << std::endl;
    }

    {
        block_search searcher{vs, 1};
        gout << "plot \'-\' u 1:2 title \'Block Search (B-Q)\' w lp" << std::endl;
        std::cout << "# Block search" << std::endl;
        std::size_t H        = 1;
        const std::size_t SZ = sizeof(data_t) + 2 * sizeof(std::size_t);
        for (std::size_t B = MinB; B <= MaxB; B++) {
            if (((1UL << (H + 1)) - 1) * SZ <= B) {
                H++;
                searcher = block_search{vs, H};
            }
            uint64_t QMax = 0;
            for (std::size_t t = 0; t < T; t++) {
                sim::initialize(B, M);  // リセット
                const data_t qx                 = qxs[t];
                [[maybe_unused]] const auto ans = searcher.lower_bound(qx);
                const auto [R, W]               = sim::cache_miss_count();
                QMax                            = std::max(QMax, R + W);
            }
            gout << std::setw(8) << B << " "
                 << std::setw(8) << QMax << std::endl;
        }
        gout << "e" << std::endl;
    }

    {
        b_tree searcher{vs, 2};
        gout << "plot \'-\' u 1:2 title \'B-Tree (B-Q)\' w lp" << std::endl;
        std::cout << "# B-Tree" << std::endl;
        std::size_t K = 2;
        for (std::size_t B = MinB; B <= MaxB; B++) {
            const std::size_t SZ = (2 * K + 1) * sizeof(data_t) + 16 * (K + 1);
            if (SZ <= B) {
                K++;
                searcher = b_tree{vs, K};
            }
            uint64_t QMax = 0;
            for (std::size_t t = 0; t < T; t++) {
                sim::initialize(B, M);  // リセット
                const data_t qx                 = qxs[t];
                [[maybe_unused]] const auto ans = searcher.lower_bound(qx);
                const auto [R, W]               = sim::cache_miss_count();
                QMax                            = std::max(QMax, R + W);
            }
            gout << std::setw(8) << B << " "
                 << std::setw(8) << QMax << std::endl;
        }
        gout << "e" << std::endl;
    }

    {
        vEB_search searcher{vs};
        gout << "plot \'-\' u 1:2 title \'vEB Search (B-Q)\' w lp" << std::endl;
        std::cout << "# vEB search" << std::endl;
        for (std::size_t B = MinB; B <= MaxB; B++) {
            uint64_t QMax = 0;
            for (std::size_t t = 0; t < T; t++) {
                sim::initialize(B, M);  // リセット
                const data_t qx                 = qxs[t];
                [[maybe_unused]] const auto ans = searcher.lower_bound(qx);
                const auto [R, W]               = sim::cache_miss_count();
                QMax                            = std::max(QMax, R + W);
            }
            gout << std::setw(8) << B << " "
                 << std::setw(8) << QMax << std::endl;
        }
        gout << "e" << std::endl;
    }

    return 0;
}
