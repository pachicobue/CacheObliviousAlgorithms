#include <cassert>
#include <iostream>

#include "data_cache.hpp"
#include "output_utility.hpp"

data_cache::data_cache(const std::size_t B, const std::size_t M) : m_page_size{B}, m_cache_size{M}, m_cacheline_num{(assert(B > 0), M / B)}, m_data_buffers(m_cacheline_num, std::vector<std::byte>(m_page_size))
{
    assert(B > 0 and M > 0);
    assert(B % 8 == 0);
    assert(M % B == 0);
    for (std::size_t i = 0; i < m_cacheline_num; i++) { m_empty_indexes.insert(i); }
}
data_cache::~data_cache() { flush(); }
std::size_t data_cache::page_size() const { return m_page_size; }
std::size_t data_cache::cache_size() const { return m_cache_size; }
std::size_t data_cache::cacheline_num() const { return m_cacheline_num; }
statistic_info data_cache::statistic() const { return m_statistic; }
void data_cache::print_summary() const
{
    std::cout << "[Cache Property]" << std::endl;
    std::cout << "- page size (B): " << m_page_size << " byte" << std::endl;
    std::cout << "- cache size(M): " << m_cache_size << " byte" << std::endl;
    std::cout << "- cacheline num: " << m_cacheline_num << " lines" << std::endl;
    std::cout << "[Statistics]" << std::endl;
    std::cout << "- disk write   : " << m_statistic.disk_write_count << " times" << std::endl;
    std::cout << "- disk read    : " << m_statistic.disk_read_count << " times" << std::endl;
}
void data_cache::debug_print() const
{
    print_summary();
    std::cout << "[Internal Status]" << std::endl;
    std::cout << "- time         : " << m_time << std::endl;
    std::cout << "- empty lines  : " << m_empty_indexes << std::endl;
    std::cout << "- pages(addr)  : " << m_pages_by_addr << std::endl;
    std::cout << "- pages(time)  : " << m_pages_by_time << std::endl;
    std::cout << "- data_buffers : " << std::endl;
    for (const auto& data_buffer : m_data_buffers) { std::cout << "                 " << data_buffer << std::endl; }
}

uintptr_t data_cache::get_page_addr(const uintptr_t addr) { return addr - (addr % m_page_size); }
std::set<page_item, page_item::addr_comparator_t>::iterator data_cache::find_by_addr(const uintptr_t page_addr) { return m_pages_by_addr.lower_bound(page_item{0, page_addr, false, 0}); }
void data_cache::delete_LRU()
{
    assert(m_pages_by_time.size() == m_cacheline_num);
    const auto item = *m_pages_by_time.begin();
    m_pages_by_time.erase(m_pages_by_time.begin()), m_pages_by_addr.erase(item);
    m_empty_indexes.insert(item.cacheline_index);
    if (item.update) {
        m_statistic.disk_write_count++;
        std::byte* disk_bytes = reinterpret_cast<std::byte*>(item.page_addr);
        for (std::size_t i = 0; i < m_page_size; i++) { disk_bytes[i] = m_data_buffers[item.cacheline_index][i]; }
    }
}
void data_cache::insert_cache(const uintptr_t page_addr, const bool update)
{
    const auto it = find_by_addr(page_addr);
    if (it != m_pages_by_addr.end() and it->page_addr == page_addr) {  // 既にcacheに存在
        auto item = *it;
        m_pages_by_addr.erase(it), m_pages_by_time.erase(item);
        item.last_used_time = m_time++;
        item.update |= update;
        m_pages_by_addr.insert(item), m_pages_by_time.insert(item);
    } else {  // ディスクから読み込み
        if (m_pages_by_time.size() == m_cacheline_num) { delete_LRU(); }
        m_statistic.disk_read_count++;
        const std::size_t index = *m_empty_indexes.begin();
        m_empty_indexes.erase(m_empty_indexes.begin());
        const auto item = page_item{m_time++, page_addr, update, index};
        m_pages_by_addr.insert(item), m_pages_by_time.insert(item);
        const std::byte* disk_bytes = reinterpret_cast<const std::byte*>(page_addr);
        for (std::size_t i = 0; i < m_page_size; i++) { m_data_buffers[index][i] = disk_bytes[i]; }
    }
    assert(m_pages_by_addr.size() == m_pages_by_time.size());
}

void data_cache::write(const uintptr_t addr, const std::byte* data, const std::size_t size)
{
    const uintptr_t end_addr = addr + static_cast<uintptr_t>(size);
    std::size_t data_index   = 0;
    for (uintptr_t page_addr = get_page_addr(addr); page_addr < end_addr; page_addr += m_page_size) {
        insert_cache(page_addr, true);
        const std::size_t index = find_by_addr(page_addr)->cacheline_index;
        for (uintptr_t i = std::max(addr, page_addr); i < std::min(end_addr, page_addr + m_page_size); i++) {
            const std::size_t cacheline_index      = static_cast<std::size_t>(i - page_addr);
            m_data_buffers[index][cacheline_index] = data[data_index++];
        }
    }
}
void data_cache::read(const uintptr_t addr, std::byte* buf, const std::size_t size)
{
    const uintptr_t end_addr = addr + static_cast<uintptr_t>(size);
    std::size_t buf_index    = 0;
    for (uintptr_t page_addr = get_page_addr(addr); page_addr < end_addr; page_addr += m_page_size) {
        insert_cache(page_addr, false);
        const std::size_t index = find_by_addr(page_addr)->cacheline_index;
        for (uintptr_t i = std::max(addr, page_addr); i < std::min(end_addr, page_addr + m_page_size); i++) {
            const std::size_t cacheline_index = static_cast<std::size_t>(i - page_addr);
            buf[buf_index++]                  = m_data_buffers[index][cacheline_index];
        }
    }
}
void data_cache::flush()
{
    std::vector<page_item> erase;
    for (const auto& page_item : m_pages_by_addr) {
        if (page_item.update) { erase.push_back(page_item); }
    }
    for (auto& page_item : erase) {
        m_statistic.disk_write_count++;
        std::byte* disk_bytes = reinterpret_cast<std::byte*>(page_item.page_addr);
        for (std::size_t i = 0; i < m_page_size; i++) { disk_bytes[i] = m_data_buffers[page_item.cacheline_index][i]; }

        m_pages_by_addr.erase(page_item), m_pages_by_time.erase(page_item);
        page_item.update = false;
        m_pages_by_addr.insert(page_item), m_pages_by_time.insert(page_item);
    }
}
