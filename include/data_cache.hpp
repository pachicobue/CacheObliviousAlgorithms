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
 * @note
 * uintptr_t型はディスクのアドレスで、T型はキャッシュ(メモリ)にコピーしたデータという気持ち
 * uintptr_tの実態はメモリ上のアドレスなので、*addrのようにアクセスできるが、ルール違反。
 * T data = disk_read<T>(addr)で読み込んだT型の情報を利用する。
 * 木構造の場合などはdataのメンバに子アドレスを含む場合があるが、この子に直接アクセスすることも禁止。読み込む。
 *
 * disk_readは単純にbyte列をコピーするので、何も考えないとstd::vectorなどの場合double freeが起きうる
 * メンバ内にデータを持たず、データ配列へのポインタのみをメンバに持つため、データ配列のコピーがされないことが原因
 * 余計なデータ型変数を作らないようにし、Tのデストラクタが呼ばれないように注意している
 * (ex) [0x1000, 0x1001,...,]：データ
 *      {0x1000} -> {0x1000} Readでポインタを複製(dataは複製されない)
 *      std::vectorのような自動でdeleteする構造体だと、0x1000からの同一データを二回freeしてしまう。
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
    void disk_write(const uintptr_t addr, const T& val)
    {
        T* buf = new T{val};
        write(addr, reinterpret_cast<std::byte*>(buf), sizeof(T));
    }
    /**
     * @brief 値読み込み
     * @param addr[in] 読み込み元のディスクアドレス
     */
    template<typename T>
    T disk_read(const uintptr_t addr)
    {
        std::byte val[sizeof(T)];
        read(addr, val, sizeof(T));
        return T{*reinterpret_cast<T*>(val)};
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
