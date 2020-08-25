#pragma once
/**
 * @file data_cache.hpp
 * @brief DCacheのシミュレータ
 * @details ディスクアクセスとキャッシュミス回数管理を行う
 */
#include <set>

#include "simulator/disk_variable.hpp"
#include "simulator/page_item.hpp"
#include "simulator/statistic_info.hpp"

class memory_bus;

/**
 * @brief DCache
 * @details 
 * Cache Oblivious Modelに従って以下のように設定
 * - ブロックサイズはB (B個のdisk_addr_t)
 * - メモリサイズはM
 * - Fully Assiciative
 * - Replacement PolicyはLRU (resource augmentaion theorem によって多くの場合正当化される)
 * @note
 * - DCacheのシミュレートというよりは、キャッシュミス回数の管理を行うクラス
 * - 今回のモデルではキャッシュミス回数だけに興味があるので、ディスクデータのコピーなどは行わない
 * - Flushを行うことでキャッシュに残っている分のdisk_write_countもカウントされる
 */
class data_cache
{
public:
    friend memory_bus;

    /**
     * @brief 統計情報
     * @note
     * - flushが直前に呼ばれる
     */
    statistic_info
        statistic();

    /**
     * @brief キャッシュに[addr,addr+size)を含むブロックを追加する
     * @param addr[in] 開始アドレス
     * @param size[in] アドレス数
     * @param update[in] 書き込み用途かどうか
     */
    void insert_address_range(const uintptr_t addr, const std::size_t size, const bool update);

    const std::size_t PageSize;
    const std::size_t CacheLineNum;
    const std::size_t CacheSize;

private:
    /**
     * @brief コンストラクタ
     * @param B[in] ブロックサイズ
     * @param M[in] キャッシュサイズ
     * @details MはBの倍数に切りあげる
     */
    data_cache(const std::size_t B, const std::size_t M);

    void flush();
    uintptr_t get_page_addr(const uintptr_t addr) const;
    std::set<page_item, page_item::addr_comparator_t>::iterator find_by_addr(const uintptr_t page_addr) const;
    void delete_LRU();
    void insert_page(const uintptr_t page_addr, const bool update);

    statistic_info m_statistic;
    uint64_t m_time = 0;
    std::set<page_item, page_item::time_comparator_t> m_pages_by_time;
    std::set<page_item, page_item::addr_comparator_t> m_pages_by_addr;
};
