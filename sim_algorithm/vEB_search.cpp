#include <algorithm>

#include "common/bit.hpp"
#include "simulator/simulator.hpp"
#include "vEB_search.hpp"

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

std::vector<std::size_t> build_orders(const std::size_t root)
{
    if (root & 1UL) {
        return std::vector<std::size_t>{root};
    }
    const std::size_t height = lsb(root) + 1;
    const std::size_t offset = root - (1UL << (height - 1));
    const std::size_t uh     = height / 2;
    const std::size_t dh     = height - uh;

    std::vector<std::size_t> orders;
    const auto top_orders = build_orders(root / (1UL << dh));
    for (const auto top_order : top_orders) {
        orders.push_back(top_order * (1UL << dh));
    }
    if (dh > 0) {
        for (std::size_t i = 1; i <= (1UL << uh); i++) {
            const auto child_orders = build_orders((1UL << (dh - 1)) * (i * 2 - 1) + offset);
            for (const auto child_order : child_orders) {
                orders.push_back(child_order);
            }
        }
    }
    return orders;
}

}  // namespace

vEB_search::vEB_search(std::vector<data_t> vs)
{
    const std::size_t N                   = vs.size();
    const std::size_t TN                  = ceil2(N + 1) - 1;
    const std::size_t ROOT                = (TN + 1) / 2;
    const std::vector<std::size_t> orders = build_orders(ROOT);  // 1-indexed
    std::vector<std::size_t> poss(TN + 1);
    for (std::size_t i = 0; i < TN; i++) {
        poss[orders[i]] = i;
    }
    std::sort(vs.begin(), vs.end());
    m_root_pos = poss[ROOT];
    for (std::size_t i = 0; i < TN; i++) {
        const std::size_t order = orders[i];
        m_xs.push_back(order > N ? data_t{Max + 1} : vs[order - 1]);
        if ((order & 1UL) == 0) {
            m_ls.push_back(poss[left(order)]);
            m_rs.push_back(poss[right(order)]);
        } else {
            m_ls.push_back(static_cast<std::size_t>(-1));
            m_rs.push_back(static_cast<std::size_t>(-1));
        }
    }
}

data_t vEB_search::lower_bound(const data_t v) const
{
    data_t ans = Max + 1;
    for (std::size_t pos = m_root_pos; pos != static_cast<std::size_t>(-1);) {
        const data_t x = sim::read(m_xs[pos]);
        if (x == v) { return v; }
        if (x < v) {
            pos = sim::read(m_rs[pos]);
        } else {
            ans = x;
            pos = sim::read(m_ls[pos]);
        }
    }
    return ans;
}
