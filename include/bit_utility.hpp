#pragma once
#include <cstddef>
constexpr std::size_t popcount(const std::size_t x) { return static_cast<std::size_t>(__builtin_popcount(x)); }
constexpr std::size_t log2(const std::size_t x) { return 32 - __builtin_clz(x); }
constexpr std::size_t lsb(const std::size_t x) { return __builtin_ffs(x) - 1; }
constexpr std::size_t clog(const std::size_t x) { return x <= 1 ? 1 : log2(x - 1) + 1; }
constexpr std::size_t ceil2(const std::size_t x) { return 1UL << clog(x); }
constexpr std::size_t floor2(const std::size_t x) { return 1UL << log2(x); }
