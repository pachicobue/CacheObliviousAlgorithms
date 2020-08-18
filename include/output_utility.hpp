#pragma once
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

namespace impl_do_not_touch {
inline void dump() { ; }
template<typename T>
inline void dump(T x) { std::cerr << x; }
template<typename T, typename... Args>
inline void dump(T x, Args... args) { std::cerr << x << ",", dump(args...); }
}  // namespace impl_do_not_touch
#define Debug(...) (std::cerr << "LINE " << __LINE__ << ": "      \
                              << "(" << #__VA_ARGS__ << ") = ("), \
                   impl_do_not_touch::dump(__VA_ARGS__), std::cerr << ")" << std::endl

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
