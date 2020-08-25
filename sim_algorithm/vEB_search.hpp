#pragma once
/**
 * @file vEB_search.hpp
 * @brief vEB Layoutを用いたCache Obliviousな二分探索
 * @note
 * - 静的なデータのみを扱う
 */
#include "config.hpp"
#include "simulator/data_cache.hpp"
#include "simulator/disk_variable.hpp"

/**
 * @brief vEB Layoutでデータを保持する構造体
 * @details
 * - LowerBound(x): データのうちx以上の最小の値を返す
 */
class vEB_search
{
public:
    /**
     * @brief コンストラクタ     
     * @param vs[in] データ配列
     */
    vEB_search(std::vector<data_t> vs);

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
