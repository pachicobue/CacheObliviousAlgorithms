#include <algorithm>

#include "block_search.hpp"
#include "common/bit.hpp"
#include "simulator/simulator.hpp"

namespace {

inline std::size_t left(const std::size_t n)
{
    const std::size_t i = lsb(n) - 1;
    return n - (1UL << i);
}

inline std::size_t right(const std::size_t n)
{
    const std::size_t i = lsb(n) - 1;
    return n + (1UL << i);
}

std::vector<std::size_t> build_orders(const std::size_t root, const std::size_t max_height)
{
    const std::size_t height = lsb(root) + 1;
    const std::size_t offset = root - (1UL << (height - 1));
    const std::size_t uh     = (height - 1) % max_height + 1;
    const std::size_t dh     = height - uh;

    std::vector<std::size_t> orders;
    for (std::size_t i = 1; i < (1UL << uh); i++) {
        orders.push_back(i * (1UL << dh) + offset);
    }
    if (dh != 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            const auto child_orders = build_orders((1UL << (dh - 1)) * (i * 2 - 1) + offset, max_height);
            for (const auto child_order : child_orders) { orders.push_back(child_order); }
        }
    }
    return orders;
}

}  // namespace

block_search::block_search(std::vector<data_t> vs, const std::size_t max_height)
{
    const std::size_t N                   = vs.size();
    const std::size_t NN                  = ceil2(N + 1) - 1;
    const std::size_t ROOT                = (NN + 1) / 2;
    const std::vector<std::size_t> orders = build_orders(ROOT, max_height);  // 1-indexed
    std::vector<std::size_t> poss(NN + 1);
    for (std::size_t i = 0; i < NN; i++) {
        poss[orders[i]] = i;
    }
    std::sort(vs.begin(), vs.end());
    m_root_index = poss[ROOT];
    for (std::size_t i = 0; i < NN; i++) {
        const std::size_t order = orders[i];
        node_t info{data_t{0}, static_cast<std::size_t>(-1), static_cast<std::size_t>(-1)};
        info.value = order > N ? Max + 1 : vs[order - 1];
        if ((order & 1UL) == 0) {
            info.left  = poss[left(order)];
            info.right = poss[right(order)];
        }
        m_nodes.push_back(disk_var<node_t>{info});
    }
}

data_t block_search::lower_bound(const data_t x) const
{
    data_t ans = Max + 1;
    for (std::size_t index = m_root_index; index != static_cast<std::size_t>(-1);) {
        const node_t info = sim::read<node_t>(m_nodes[index]);
        if (info.value == x) { return x; }
        if (info.value < x) {
            index = info.right;
        } else {
            ans   = std::min(ans, info.value);
            index = info.left;
        }
    }
    return ans;
}
