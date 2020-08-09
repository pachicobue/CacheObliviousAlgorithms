#pragma once
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T, A>& v)
{
    os << "[";
    for (const auto& e : v) { os << e << ","; }
    return (os << "]");
}
template<typename T, typename C, typename A>
inline std::ostream& operator<<(std::ostream& os, const std::set<T, C, A>& v)
{
    os << "[";
    for (const auto& e : v) { os << e << ","; }
    return (os << "]");
}
inline std::ostream& operator<<(std::ostream& os, const std::byte v)
{
    constexpr const char* chars = "0123456789ABCDEF";
    return (os << "0x" << chars[static_cast<int>(v) >> 4] << chars[static_cast<int>(v) & (0x0F)]);
}
template<typename T>
inline std::string hex_str(T v)
{
    constexpr const char* chars = "0123456789ABCDEF";
    std::string str;
    for (; v > 0; v >>= 4) { str.push_back(chars[v & 0x0F]); }
    std::reverse(str.begin(), str.end());
    return ("0x" + str);
}
