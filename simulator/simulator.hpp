#pragma once

#include "memory_bus.hpp"

namespace sim {

// 読み書きはこいつを通じて行う
extern memory_bus* g_bus_ptr;

/**
 * @brief 再構築
 * @param B[in]
 * @param M[in]
 * @details キャッシュ特性を変更してresetする
 */
void initialize(const std::size_t B, const std::size_t M);

/**
 * @brief 値書き込み
 * @param addr[in] 書き込み先のディスク変数
 * @param val[in] 書きこむデータ
 */
template<typename T>
void write(disk_var<T>& dv, const T& val)
{
    g_bus_ptr->write<T>(dv, val);
}

/**
 * @brief 値読み込み
 * @param dv[in] 読み込みたいディスク変数
 */
template<typename T>
const T& read(const disk_var<T>& dv)
{
    return g_bus_ptr->read<T>(dv);
}

/**
 * @brief 統計情報
 */
statistic_info cache_miss_count();

}  // namespace sim
