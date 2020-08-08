#pragma once
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "output_utility.hpp"
/**
 * @brief キャッシュに入れるページ情報
 * @detail 
 * - last_used_time：最後に触った時間
 * - disk_addr：ページの先頭のディスクアドレス
 * - update：書き込みをするか
 * - cacheline_index：何番目のキャッシュラインか
 */
struct page_item
{
    uint64_t last_used_time           = 0;
    const uintptr_t page_addr         = 0;
    bool update                       = false;
    const std::size_t cacheline_index = 0;
    struct time_comparator_t
    {
        bool operator()(const page_item& item1, const page_item& item2) const { return item1.last_used_time < item2.last_used_time; }
    };
    struct addr_comparator_t
    {
        bool operator()(const page_item& item1, const page_item& item2) const { return item1.page_addr < item2.page_addr; }
    };
    friend std::ostream& operator<<(std::ostream& os, const page_item& item) { return (os << "{time=" << item.last_used_time << ",addr=" << hex_str(item.page_addr) << ",update=" << item.update << ", index=" << item.cacheline_index << "}"); }
};
