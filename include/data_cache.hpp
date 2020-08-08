#pragma once
#include <map>
#include <set>
#include <vector>

#include "page_item.hpp"
#include "statistic_info.hpp"
/**
 * @brief Data Cacheの仮想化
 * @details
 * - ブロックサイズはB (B個のuintptr_t)
 * - メモリサイズはM
 * - Fully Assiciative
 * - Replacement PolicyはLRU
 */
class data_cache
{
public:
    /**
     * @brief コンストラクタ
     * @param B[in] ページサイズ
     * @param M[in] メモリサイズ
     */
    data_cache(const std::size_t B, const std::size_t M);
    /**
     * @brief デストラクタ
     */
    ~data_cache();
    /**
     * @brief 値書き込み
     * @param addr[in] 書き込み先のディスクアドレス
     * @param val[in] 書きこむデータ
     */
    template<typename T>
    void disk_write(const uintptr_t addr, const T& val) { write(addr, reinterpret_cast<const std::byte*>(&val), sizeof(val)); }
    /**
     * @brief 値読み込み
     * @param addr[in] 読み込み元のディスクアドレス
     */
    template<typename T>
    T disk_read(const uintptr_t addr)
    {
        T val;
        read(addr, reinterpret_cast<std::byte*>(&val), sizeof(T));
        return val;
    }
    /**
     * @brief Flush操作
     * @details update状態で残ったページを実際に書きこむ
     */
    void flush();

    std::size_t page_size() const;
    std::size_t cache_size() const;
    std::size_t cacheline_num() const;
    statistic_info statistic() const;

    void print_summary() const;
    void debug_print() const;

private:
    uintptr_t get_page_addr(const uintptr_t addr);
    std::set<page_item, page_item::addr_comparator_t>::iterator find_by_addr(const uintptr_t page_addr);
    void delete_LRU();
    void insert_cache(const uintptr_t page_addr, const bool update);
    void write(const uintptr_t addr, const std::byte* data, const std::size_t size);
    void read(const uintptr_t addr, std::byte* buf, const std::size_t size);

    const std::size_t m_page_size;
    const std::size_t m_cache_size;
    const std::size_t m_cacheline_num;

    statistic_info m_statistic;
    uint64_t m_time = 0;

    std::set<page_item, page_item::time_comparator_t> m_pages_by_time;
    std::set<page_item, page_item::addr_comparator_t> m_pages_by_addr;
    std::set<std::size_t> m_empty_indexes;
    std::vector<std::vector<std::byte>> m_data_buffers;
};
