#pragma once
#include <cassert>
#include <set>
#include <vector>

#include "output_utility.hpp"
#include "page_item.hpp"
#include "statistic_info.hpp"
/**
 * @brief Data Cacheの仮想化
 * @details
 * - ブロックサイズはB (B個のuintptr_t)
 * - メモリサイズはM
 * - Fully Assiciative
 * - Replacement PolicyはLRU
 * @note
 * [1] uintptr_t型はディスクのアドレスで、T型はキャッシュ(メモリ)にコピーしたデータという気持ち
 *     uintptr_tの実態はメモリ上のアドレスなので、*addrのようにアクセスできるが、ルール違反
 *     T data = disk_read<T>(addr)で読み込んだT型の情報を利用する。
 *     木構造の場合などはdataのメンバに子アドレスを含む場合があるが、この子に直接アクセスすることも禁止
 *
 * [2] 今回のモデルではディスクへのアクセス回数だけに興味があるので、キャッシュへのデータのコピーは行わない
 *     メモリリークが発生したり、またそれを回避するために設計上余計な制約が必要とされるため
 *     データの反映は即座に行われるが、disk_write_count,disk_read_countは追い出しのタイミングやキャッシュミスのタイミングで加算する
 *
 * [3] 多分Cache Obliviousの設定ではwrite-back方式かwrite-through方式かは定義されていない(disk_read_countだけを考慮？)
 *     disk_write_countがdisk_read_countに対して大きくなるのが嫌だったのでwrite-back方式にしたかった
 */
template<std::size_t B, std::size_t M>
class data_cache
{
public:
    /**
     * @brief コンストラクタ
     */
    data_cache()
    {
        static_assert(B > 0, "B(page size) should be positive.");
        static_assert(M % B == 0, "M(cache size) should be multiple of B.");
        for (std::size_t i = 0; i < CacheLineNum; i++) { m_empty_indexes.insert(i); }
    }
    /**
     * @brief 値書き込み
     * @param addr[in] 書き込み先のディスクアドレス
     * @param val[in] 書きこむデータ
     */
    template<typename T>
    void disk_write(const uintptr_t addr, const T& val) { write(addr, reinterpret_cast<const std::byte*>(&val), sizeof(T)); }
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
     * @brief 統計情報
     */
    statistic_info statistic() const { return m_statistic; }
    /**
     * @brief 統計情報などの出力
     */
    void print_summary() const
    {
        std::cout << "[Cache Property]" << std::endl;
        std::cout << "- page size (B): " << PageSize << " byte" << std::endl;
        std::cout << "- cache size(M): " << CacheSize << " byte" << std::endl;
        std::cout << "- cacheline num: " << CacheLineNum << " lines" << std::endl;
        std::cout << "[Statistics]" << std::endl;
        std::cout << "- disk write   : " << m_statistic.disk_write_count << " times" << std::endl;
        std::cout << "- disk read    : " << m_statistic.disk_read_count << " times" << std::endl;
    }
    /**
     * @brief デバッグ出力
     */
    void debug_print() const
    {
        print_summary();
        std::cout << "[Internal Status]" << std::endl;
        std::cout << "- time         : " << m_time << std::endl;
        std::cout << "- empty lines  : " << m_empty_indexes << std::endl;
        std::cout << "- pages(addr)  : " << m_pages_by_addr << std::endl;
        std::cout << "- pages(time)  : " << m_pages_by_time << std::endl;
    }
    /**
     * @brief Flush
     * @details 統計情報を見る前に呼ぶとdisk_write_countが正確になる
     */
    void flush()
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
    /**
     * @brief キャッシュを初期化する
     */
    void reset()
    {
        m_statistic.disk_write_count = 0;
        m_statistic.disk_read_count  = 0;
        m_time                       = 0;
        m_pages_by_time.clear();
        m_pages_by_addr.clear();
        for (std::size_t i = 0; i < CacheLineNum; i++) { m_empty_indexes.insert(i); }
    }

    static constexpr std::size_t PageSize     = B;
    static constexpr std::size_t CacheSize    = M;
    static constexpr std::size_t CacheLineNum = M / B;

private:
    static uintptr_t get_page_addr(const uintptr_t addr) { return addr - (addr % PageSize); }
    std::set<page_item, page_item::addr_comparator_t>::iterator find_by_addr(const uintptr_t page_addr) const { return m_pages_by_addr.lower_bound(page_item{0, page_addr, false, 0}); }
    void delete_LRU()
    {
        assert(m_pages_by_time.size() == CacheLineNum);
        const auto item = *m_pages_by_time.begin();
        m_pages_by_time.erase(m_pages_by_time.begin()), m_pages_by_addr.erase(item);
        m_empty_indexes.insert(item.cacheline_index);
        if (item.update) { m_statistic.disk_write_count++; }
    }
    void insert_cache(const uintptr_t page_addr, const bool update)
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
    void write(const uintptr_t addr, const std::byte* data, const std::size_t size)
    {
        const uintptr_t end_addr = addr + static_cast<uintptr_t>(size);
        for (uintptr_t page_addr = get_page_addr(addr); page_addr < end_addr; page_addr += PageSize) { insert_cache(page_addr, true); }
        std::byte* disk_data = reinterpret_cast<std::byte*>(addr);
        for (std::size_t i = 0; i < size; i++) { disk_data[i] = data[i]; }
    }
    void read(const uintptr_t addr, std::byte* buf, const std::size_t size)
    {
        const uintptr_t end_addr = addr + static_cast<uintptr_t>(size);
        for (uintptr_t page_addr = get_page_addr(addr); page_addr < end_addr; page_addr += PageSize) { insert_cache(page_addr, false); }
        std::byte* disk_data = reinterpret_cast<std::byte*>(addr);
        for (std::size_t i = 0; i < size; i++) { buf[i] = disk_data[i]; }
    }

    statistic_info m_statistic;
    uint64_t m_time = 0;

    std::set<page_item, page_item::time_comparator_t> m_pages_by_time;
    std::set<page_item, page_item::addr_comparator_t> m_pages_by_addr;
    std::set<std::size_t> m_empty_indexes;
};
