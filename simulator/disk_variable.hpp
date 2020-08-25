#pragma once
/**
 * @file disk_variable.hpp
 * @brief ディスク上の値を表す構造体
 * @note
 * - このシミュレータでは実際にディスク読み込みなどはせず、実態はメモリ読み込みを行っている。
 *   ディスクアクセスという気持ちでメモリアクセスを行うためのラッパー。
 * - 基本的にdata_cacheを通して読み書きを行う
 *   直接いじることも可能だが、キャッシュを介さないので前計算パート以外で使っちゃダメ。
 */
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

class memory_bus;

using disk_addr_t = uintptr_t;

/**
 * @brief ディスク上の変数
 */
template<typename T>
class disk_var
{
    friend memory_bus;

public:
    /**
     * @brief コンストラクタ
     * @param args[in] 引数
     */
    template<typename... Args>
    disk_var(Args... args) : m_val{args...} {}

    /**
     * @brief 変数のディスクアドレス
     */
    disk_addr_t addr() const
    {
        return reinterpret_cast<disk_addr_t>(&m_val);
    }

    /**
     * @brief 直接参照
     * @note
     * - 基本的には前計算でのみ使う
     */
    T& illegal_ref() { return m_val; }

    /**
     * @brief 直接参照
     * @note
     * - 基本的には前計算でのみ使う
     */
    const T& illegal_ref() const { return m_val; }

private:
    T m_val;
};
