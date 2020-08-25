#pragma once
/**
 * @file binary_search.hpp
 * @brief ただの二分探索
 * @note
 * - 静的なデータのみを扱う
 */
#include "config.hpp"
#include "simulator/data_cache.hpp"
#include "simulator/disk_variable.hpp"

/**
 * @brief 昇順でデータを保持する構造体
 * @details
 * - LowerBound(x): データのうちx以上の最小の値を返す
 */
class binary_search
{
public:
    /**
     * @brief コンストラクタ     
     * @param vs[in] データ配列
     */
    binary_search(std::vector<data_t> vs);

    /**
     * @brief LowerBoundクエリ
     * @param x[in] 
     */
    data_t lower_bound(const data_t x) const;

private:
    std::vector<disk_var<data_t>> m_datas;
};
