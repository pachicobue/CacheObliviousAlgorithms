#pragma once
/**
 * @file disk_addr.hpp
 * @brief ディスクアドレス型
 * @note
 * - このシミュレータでは実際にディスク読み込みなどはせず、実態はメモリ読み込みを行っている
 *   ディスクアクセスという気持ちでメモリアクセスを行うための構造体
 */

#include <cstdint>
class data_cache;

/**
 * @brief ディスク上のアドレスというつもり
 * @note data_cache以外のポインタアクセスを禁止
 */
class disk_addr_t
{
    friend class data_cache;

public:
    disk_addr_t() = default;
    explicit disk_addr_t(const uintptr_t addr) : m_addr{addr} {}

private:
    uintptr_t m_addr = 0x00000000;
};
