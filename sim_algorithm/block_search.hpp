#pragma once
/**
 * @file block_search.hpp
 * @brief Block Layoutを用いたCache Awareな二分探索
 * @note
 * - 静的なデータのみを扱う
 * - B-木に類似している
 */
#include "config.hpp"
#include "simulator/data_cache.hpp"
#include "simulator/disk_variable.hpp"

/**
 * @brief Block Layoutでデータを保持する構造体
 * @details
 * - LowerBound(x): データのうちx以上の最小の値を返す
 */
class block_search
{
public:
    /**
     * @brief コンストラクタ     
     * @param vs[in] データ配列
     * @param max_height[in] ブロックの最大高さ
     */
    block_search(std::vector<data_t> vs, const std::size_t max_height);

    /**
     * @brief LowerBoundクエリ
     * @param x[in] 
     */
    data_t lower_bound(const data_t x) const;

private:
    struct node_t
    {
        data_t value;
        std::size_t left;
        std::size_t right;
    };
    std::size_t m_root_index;
    std::vector<disk_var<node_t>> m_nodes;
};
