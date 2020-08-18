#include <cassert>
#include <vector>

#include "data_cache.hpp"
#include "output_utility.hpp"

data_cache::data_cache(const std::size_t B, const std::size_t M) : PageSize{B}, CacheLineNum{(M + B - 1) / B}, CacheSize{PageSize * CacheLineNum}
{
    for (std::size_t i = 0; i < CacheLineNum; i++) { m_empty_indexes.insert(i); }
}

statistic_info data_cache::statistic() const
{
    return m_statistic;
}

void data_cache::print_summary() const
{
    std::cout << "[Cache Property]" << std::endl;
    std::cout << "- page size (B): " << PageSize << " byte" << std::endl;
    std::cout << "- cache size(M): " << CacheSize << " byte" << std::endl;
    std::cout << "- cacheline num: " << CacheLineNum << " lines" << std::endl;
    std::cout << "[Statistics]" << std::endl;
    std::cout << "- disk write   : " << m_statistic.disk_write_count << " times" << std::endl;
    std::cout << "- disk read    : " << m_statistic.disk_read_count << " times" << std::endl;
}

void data_cache::debug_print() const
{
    std::cout << "[Cache Property]" << std::endl;
    std::cout << "- page size (B): " << PageSize << " byte" << std::endl;
    std::cout << "- cache size(M): " << CacheSize << " byte" << std::endl;
    std::cout << "- cacheline num: " << CacheLineNum << " lines" << std::endl;
    std::cout << "[Statistics]" << std::endl;
    std::cout << "- disk write   : " << m_statistic.disk_write_count << " times" << std::endl;
    std::cout << "- disk read    : " << m_statistic.disk_read_count << " times" << std::endl;
    std::cout << "[Internal Status]" << std::endl;
    std::cout << "- time         : " << m_time << std::endl;
    std::cout << "- empty lines  : " << m_empty_indexes << std::endl;
    std::cout << "- pages(addr)  : " << m_pages_by_addr << std::endl;
    std::cout << "- pages(time)  : " << m_pages_by_time << std::endl;
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

void data_cache::reset()
{
    m_statistic.disk_write_count = 0;
    m_statistic.disk_read_count  = 0;
    m_time                       = 0;
    m_pages_by_time.clear();
    m_pages_by_addr.clear();
    for (std::size_t i = 0; i < CacheLineNum; i++) { m_empty_indexes.insert(i); }
}

uintptr_t data_cache::get_page_addr(const uintptr_t addr) const
{
    return addr - (addr % PageSize);
}

std::set<page_item, page_item::addr_comparator_t>::iterator data_cache::find_by_addr(const uintptr_t page_addr) const
{
    return m_pages_by_addr.lower_bound(page_item{0, page_addr, false, 0});
}

void data_cache::delete_LRU()
{
    assert(m_pages_by_time.size() == CacheLineNum);
    const auto item = *m_pages_by_time.begin();
    m_pages_by_time.erase(m_pages_by_time.begin()), m_pages_by_addr.erase(item);
    m_empty_indexes.insert(item.cacheline_index);
    if (item.update) { m_statistic.disk_write_count++; }
}

void data_cache::insert_cache(const uintptr_t page_addr, const bool update)
{
    const auto it = find_by_addr(page_addr);
    if (it != m_pages_by_addr.end() and it->page_addr == page_addr) {
        auto item = *it;
        m_pages_by_addr.erase(it), m_pages_by_time.erase(item);
        item.last_used_time = m_time++;
        item.update |= update;
        m_pages_by_addr.insert(item), m_pages_by_time.insert(item);
    } else {
        if (m_pages_by_time.size() == CacheLineNum) { delete_LRU(); }
        m_statistic.disk_read_count++;
        const std::size_t index = *m_empty_indexes.begin();
        m_empty_indexes.erase(m_empty_indexes.begin());
        const auto item = page_item{m_time++, page_addr, update, index};
        m_pages_by_addr.insert(item), m_pages_by_time.insert(item);
    }
    assert(m_pages_by_addr.size() == m_pages_by_time.size());
}

void data_cache::write(const uintptr_t addr, const std::byte* data, const std::size_t size)
{
    const uintptr_t end_addr = addr + static_cast<uintptr_t>(size);
    for (uintptr_t page_addr = get_page_addr(addr); page_addr < end_addr; page_addr += PageSize) { insert_cache(page_addr, true); }
    std::byte* disk_data = reinterpret_cast<std::byte*>(addr);
    for (std::size_t i = 0; i < size; i++) { disk_data[i] = data[i]; }
}

void data_cache::read(const uintptr_t addr, std::byte* buf, const std::size_t size)
{
    const uintptr_t end_addr = addr + static_cast<uintptr_t>(size);
    for (uintptr_t page_addr = get_page_addr(addr); page_addr < end_addr; page_addr += PageSize) { insert_cache(page_addr, false); }
    std::byte* disk_data = reinterpret_cast<std::byte*>(addr);
    for (std::size_t i = 0; i < size; i++) { buf[i] = disk_data[i]; }
}
