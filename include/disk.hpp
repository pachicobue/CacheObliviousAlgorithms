#pragma once
/**
 * @file disk_utility
 * @brief ディスク操作系
 * @details 
 * ディスク(=非キャッシュ)からの読み込みを模倣するための関数
 * このシミュレータでは実際にディスク読み込みなどはせず、実態はメモリ読み込みを行っている
 * ディスクアクセスという気持ちでメモリアクセスするためのラッパー
 */
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

#include "output_utility.hpp"

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
    friend std::ostream& operator<<(std::ostream& os, const disk_addr_t& addr) { return (os << hex_str(addr.m_addr)); }

private:
    uintptr_t m_addr = 0x0000;
};

/**
 * @brief ディスク上の変数(値系)
 * @note 
 * - intなどをメンバに持つ構造体にも適用可能
 *   正確にはstruct* に全ての情報が含まれているような構造体
 * - std::vector,std::stringなどは例外で、これを使うのは反則(disk_vectorを使うようにする)
 *   std::vectorのポインタ自体には配列情報が含まれていないのに、アクセスできるというズルができてしまう
 * - 一応固定長配列はOKだが、disk_arrayを使ったほうがわかりやすい気がする
 */
template<typename T>
class disk_val
{
public:
    /**
     * @brief コンストラクタ
     */
    disk_val() = default;

    /**
     * @brief 変数のディスクアドレス
     */
    disk_addr_t addr() const { return disk_addr_t{reinterpret_cast<uintptr_t>(&m_val)}; }

    friend std::ostream& operator<<(std::ostream& os, const disk_val& v) { return (os << v.m_val); }

private:
    T m_val;
};

/**
 * @brief ディスク上の固定長配列
 */
template<typename T, std::size_t N>
class disk_array
{
public:
    /**
     * @brief コンストラクタ
     */
    disk_array() = default;

    /**
     * @brief i番目の要素のアドレス
     * @param i[in] index    
     */
    disk_addr_t addr(const std::size_t i) const { return disk_addr_t{reinterpret_cast<uintptr_t>(&m_vals[i])}; }

    friend std::ostream& operator<<(std::ostream& os, const disk_array& dvs) { return (os << dvs.m_vals); }

private:
    std::array<T, N> m_vals;
    std::size_t m_size = 0;
};

/**
 * @brief ディスク上の可変長配列
 */
template<typename T>
class disk_vector
{
public:
    /**
     * @brief コンストラクタ
     */
    disk_vector() = default;

    /**
     * @brief コンストラクタ
     * @param sz[in] 初期サイズ
     */
    disk_vector(const std::size_t sz) : m_vals(sz), m_size{sz} {}

    /**
     * @brief i番目の要素のアドレス
     * @param i[in] index    
     */
    disk_addr_t addr(const std::size_t i) const { return disk_addr_t{reinterpret_cast<uintptr_t>(&m_vals[i])}; }

    /**
     * @brief サイズ変数へのアドレス
     */
    disk_addr_t size_addr() const { return disk_addr_t{reinterpret_cast<uintptr_t>(&m_size)}; }

    /**
     * @brief push_back
     * @param val[in] 追加する値
     * @note 
     * - push_backに必要なディスクアクセス回数などは無視
     *   モデルではディスク容量は無限であり、必要の無い操作であるため(初めに確保すればいい)
     *   現実(シミュレータ)のメモリ節約のために提供される関数という位置づけ
     * - 再配置のせいで、理論よりキャッシュミスは多くなりうる     
     *   しょうがないのであまり気にしない
     */
    void push_back(const T& val)
    {
        m_vals.push_back(val);
        m_size++;
    }

    /**
     * @brief pop_back
     * @note 
     * - pop_backに必要なディスクアクセス回数などは無視
     */
    void pop_back()
    {
        m_vals.pop_back();
        m_size--;
    }

    /**
     * @brief resize
     * @note 
     * - resizeに必要なディスクアクセス回数などは無視
     */
    void resize(const std::size_t sz)
    {
        m_vals.resize(sz);
        m_size = sz;
    }

    friend std::ostream& operator<<(std::ostream& os, const disk_vector& dvs) { return (os << dvs.m_vals); }

private:
    std::vector<T> m_vals;
    std::size_t m_size = 0;
};
