#include <cassert>

#include "data_cache.hpp"
namespace {
using Item = std::pair<std::uintptr_t, std::uint64_t>;
using Meti = std::pair<std::uint64_t, std::uintptr_t>;
}  // anonymous namespace

data_cache::data_cache(const std::size_t B_, const std::size_t M_) : m_block_size{B_}, m_cache_size{M_}, m_time{0} {}

bool data_cache::contain(const std::uintptr_t addr) const
{
    const auto it = m_items.lower_bound(Item{addr, 0});
    return it != m_items.end() and it->first == addr;
}

void data_cache::insert(const std::vector<std::uintptr_t>& addr_block)
{
    assert(addr_block.size() == block_size());
    for (const std::uintptr_t addr : addr_block) {
        if (contain(addr)) {
            const auto it     = m_items.lower_bound(Item{addr, 0});
            const uint64_t ts = it->second;
            m_items.erase(Item{addr, ts}), m_metis.erase(Meti{ts, addr});
            m_items.insert(Item{addr, m_time}), m_metis.insert(Meti{m_time, addr});
        } else {
            if (m_items.size() == m_cache_size) {
                const auto it                 = m_metis.begin();
                const auto [lru_addr, lru_ts] = *it;
                m_items.erase(Item{lru_addr, lru_ts}), m_metis.erase(Meti{lru_ts, lru_addr});
            }
            m_items.insert(Item{addr, m_time}), m_metis.insert(Meti{m_time, addr});
        }
    }
    m_time++;
}

void data_cache::erase(const std::uintptr_t addr)
{
    if (not contain(addr)) { return; }
    const auto it     = m_items.lower_bound(Item{addr, 0});
    const uint64_t ts = it->second;
    m_items.erase(Item{addr, ts}), m_metis.erase(Meti{ts, addr});
}

std::size_t data_cache::block_size() const { return m_block_size; }

std::size_t data_cache::cache_size() const { return m_cache_size; }
