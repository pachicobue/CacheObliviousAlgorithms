#include <algorithm>

#include "binary_search.hpp"
#include "simulator/simulator.hpp"

binary_search::binary_search(std::vector<data_t> vs)
{
    std::sort(vs.begin(), vs.end());
    vs.push_back(Max + 1);
    for (const auto v : vs) { m_datas.push_back(disk_var<data_t>{v}); }
}

data_t binary_search::lower_bound(const data_t x) const
{
    int inf = -1, sup = static_cast<int>(m_datas.size());
    while (sup - inf > 1) {
        const std::size_t mid = static_cast<std::size_t>(inf + sup) / 2;
        const data_t v        = sim::read<data_t>(m_datas[mid]);
        if (v == x) { return v; }
        (v < x ? inf : sup) = static_cast<int>(mid);
    }
    return sim::read<data_t>(m_datas[sup]);
}
