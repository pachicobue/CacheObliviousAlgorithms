#pragma once
/**
 * @file output.hpp
 * @brief デバッグ出力用関数群
 */
#include <array>
#include <iostream>
#include <vector>

/**
 * @brief 変数の出力
 * @details dump(a,b,c); とすると変数a,b,cの値が空白区切りで出力される
 */
inline void dump() {}
template<typename T>
inline void dump(T x)
{
    std::cerr << x;
}
template<typename T, typename... Args>
inline void dump(T x, Args... args)
{
    std::cerr << x << ",";
    dump(args...);
}

/**
 * @brief デバッグ出力
 * @details Debug(a,b,c); とすると変数名＋行番号と共にa,b,cの値が出力される(便利)
 */
#define Debug(...) (std::cerr << "LINE " << __LINE__ << ": "      \
                              << "(" << #__VA_ARGS__ << ") = ("), \
                   dump(__VA_ARGS__),                             \
                   std::cerr << ")" << std::endl

/**
 * @brief 各コンテナの出力
 */
template<typename T, std::size_t N>
inline std::ostream& operator<<(std::ostream& os, const std::array<T, N>& v)
{
    os << "[";
    for (const auto& e : v) { os << e << ","; }
    return (os << "]" << std::endl);
}
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T, A>& v)
{
    os << "[";
    for (const auto& e : v) { os << e << ","; }
    return (os << "]");
}
