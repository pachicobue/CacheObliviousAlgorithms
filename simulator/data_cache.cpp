#include <cassert>
#include <vector>

#include "simulator/data_cache.hpp"

data_cache::data_cache(const std::size_t B, const std::size_t M) : PageSize{B}, CacheLineNum{(M + B - 1) / B}, CacheSize{PageSize * CacheLineNum} {}

statistic_info data_cache::statistic()
{
    flush();
    return m_statistic;
}

void data_cache::insert_address_range(const uintptr_t addr, const std::size_t size, const bool update)
{
    const uintptr_t end_addr = addr + static_cast<uintptr_t>(size);
    for (uintptr_t page_addr = get_page_addr(addr); page_addr < end_addr; page_addr += PageSize) { insert_page(page_addr, update); }
}

void data_cache::flush()
{
    std::vector<page_item> erase;
    for (const auto& page_item : m_pages_by_addr) {
        if (page_item.update) { erase.push_back(page_item); }
    }
    for (auto& page_item : erase) {
        m_statistic.disk_write_count++;
        m_pages_by_addr.erase(page_item), m_pages_by_time.erase(page_item);
        page_item.update = false;
        m_pages_by_addr.insert(page_item), m_pages_by_time.insert(page_item);
    }
}

uintptr_t data_cache::get_page_addr(const uintptr_t addr) const
{
    return addr - (addr % PageSize);
}

std::set<page_item, page_item::addr_comparator_t>::iterator data_cache::find_by_addr(const uintptr_t page_addr) const
{
    return m_pages_by_addr.lower_bound(page_item{0, page_addr, false});
}

void data_cache::delete_LRU()
{
    const auto item = *m_pages_by_time.begin();
    m_pages_by_time.erase(m_pages_by_time.begin()), m_pages_by_addr.erase(item);
    if (item.update) { m_statistic.disk_write_count++; }
}

void data_cache::insert_page(const uintptr_t page_addr, const bool update)
{
    const auto it = find_by_addr(page_addr);
    if (it != m_pages_by_addr.end() and it->page_addr == page_addr) {
        auto item = *it;
        m_pages_by_addr.erase(it), m_pages_by_time.erase(item);
        item.last_used_time = ++m_time;
        item.update |= update;
        m_pages_by_addr.insert(item), m_pages_by_time.insert(item);
    } else {
        if (m_pages_by_time.size() == CacheLineNum) { delete_LRU(); }
        m_statistic.disk_read_count++;
        const auto item = page_item{++m_time, page_addr, update};
        m_pages_by_addr.insert(item), m_pages_by_time.insert(item);
    }
}
