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
    block_search(std::vector<data_t> vs, const std::size_t block_height);

    /**
     * @brief LowerBoundクエリ
     * @param v[in] 
     */
    data_t lower_bound(const data_t v) const;

private:
    std::size_t m_root_pos;
    std::vector<disk_var<std::size_t>> m_ls, m_rs;
    std::vector<disk_var<data_t>> m_xs;
};
