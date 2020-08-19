#pragma once
#include <set>

#include "disk.hpp"
#include "page_item.hpp"
#include "statistic_info.hpp"

/**
 * @brief Data Cacheの仮想化
 * @details
 * - ブロックサイズはB (B個のdisk_addr_t)
 * - メモリサイズはM
 * - Fully Assiciative
 * - Replacement PolicyはLRU (resource augmentaion theorem によって多くの場合正当化される)
 * @note
 * - disk_addr_tの中身にアクセスできる唯一のクラス
 *   disk_addr_t <-> uintptr_t への変換を行って管理
 * - 今回のモデルではキャッシュミス回数だけに興味があるので、キャッシュへのデータのコピーは行わない
 *   メモリリークが発生したり、またそれを回避するために設計上余計な制約が必要とされるため
 *   データの反映は即座に行われるが、disk_write_count,disk_read_countは追い出しのタイミングやキャッシュミスのタイミングで加算する
 */
class data_cache
{
public:
    /**
     * @brief コンストラクタ
     * @param B[in] ブロックサイズ
     * @param M[in] キャッシュサイズ
     * @details MはBの倍数に切りあげる
     */
    data_cache(const std::size_t B, const std::size_t M);

    /**
     * @brief 値書き込み
     * @param addr[in] 書き込み先のディスクアドレス
     * @param val[in] 書きこむデータ
     */
    template<typename T>
    void disk_write(const disk_addr_t addr, const T& val)
    {
        insert_address_range(addr.m_addr, sizeof(T), true);
        disk_write_raw(addr, val);
    }

    /**
     * @brief 値読み込み
     * @param addr[in] 読み込み元のディスクアドレス
     */
    template<typename T>
    T disk_read(const disk_addr_t addr)
    {
        insert_address_range(addr.m_addr, sizeof(T), false);
        return disk_read_raw<T>(addr);
    }

    /**
     * @brief 値書き込み
     * @param addr[in] 書き込み先のディスクアドレス
     * @param val[in] 書きこむデータ
     * @details キャッシュを介さない生アクセス
     * @note 
     * - キャッシュ効率に興味ない部分でのシミュレーション高速化のための関数
     *   例えば静的データに対する検索クエリ効率を比較する際でのB-木の構築や、データをディスクに並べておく部分などはこれを使う
     *   計算量に関わる部分で使うのは反則
     */
    template<typename T>
    static void disk_write_raw(const disk_addr_t addr, const T& val)
    {
        *reinterpret_cast<T*>(addr.m_addr) = val;
    }

    /**
     * @brief 値読み込み
     * @param addr[in] 読み込み元のディスクアドレス
     * @details キャッシュを介さない生アクセス
     * @note 
     * - キャッシュ効率に興味ない部分でのシミュレーション高速化のための関数
     *   例えば静的データに対する検索クエリ効率を比較する際でのB-木の構築や、データをディスクに並べておく部分などはこれを使う
     *   計算量に関わる部分で使うのは反則
     */
    template<typename T>
    static T disk_read_raw(const disk_addr_t addr)
    {
        return *reinterpret_cast<const T*>(addr.m_addr);
    }

    /**
     * @brief 統計情報
     */
    statistic_info statistic() const;

    /**
     * @brief 統計情報などの出力
     */
    void print_summary() const;

    /**
     * @brief デバッグ出力
     */
    void debug_print() const;

    /**
     * @brief Flush
     * @details 統計情報を見る前に呼ぶとdisk_write_countが正確になる
     */
    void flush();

    /**
     * @brief キャッシュを初期化する
     */
    void reset();

    const std::size_t PageSize;
    const std::size_t CacheLineNum;
    const std::size_t CacheSize;

private:
    uintptr_t get_page_addr(const uintptr_t addr) const;
    std::set<page_item, page_item::addr_comparator_t>::iterator find_by_addr(const uintptr_t page_addr) const;
    void delete_LRU();
    void insert_page(const uintptr_t page_addr, const bool update);
    void insert_address_range(const uintptr_t addr, const std::size_t size, const bool update);

    statistic_info m_statistic;
    uint64_t m_time = 0;
    std::set<page_item, page_item::time_comparator_t> m_pages_by_time;
    std::set<page_item, page_item::addr_comparator_t> m_pages_by_addr;
};
