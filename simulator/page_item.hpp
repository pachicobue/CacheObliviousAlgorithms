#pragma once
/**
 * @file page_item.hpp
 * @brief DCacheが管理するページ情報
 */
#include <cstdint>

/**
 * @brief ページ情報
 * @detail 
 * - last_used_time：最後に触った時間
 * - disk_addr：ページの先頭のディスクアドレス
 * - update：書き込みをするか
 */
struct page_item
{
    uint64_t last_used_time   = 0;
    const uintptr_t page_addr = 0;
    bool update               = false;

    /**     
     * @brief 比較関数(last_used_time)
     * @note
     * - LRUを求める際に使う
     */
    struct time_comparator_t
    {
        bool operator()(const page_item& item1, const page_item& item2) const
        {
            return item1.last_used_time < item2.last_used_time;
        }
    };

    /**     
     * @brief 比較関数(page_addr)
     * @note
     * - アクセス先を含むページが存在するかの検索に使う
     */
    struct addr_comparator_t
    {
        bool operator()(const page_item& item1, const page_item& item2) const
        {
            return item1.page_addr < item2.page_addr;
        }
    };
};
