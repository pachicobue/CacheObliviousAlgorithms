#pragma once
/**
 * @file bit.hpp
 * @brief ビット操作系
 */
#include <cstddef>

/**
 * @brief floor(log_2(x))
 * @details MSBと等しい
 * @note
 * - 0を渡しちゃだめ
 */
constexpr std::size_t lg(const std::size_t x)
{
    return 32 - __builtin_clz(x);
}

/**
 * @brief ceil(log_2(x))
 */
constexpr std::size_t clg(const std::size_t x)
{
    return x <= 1 ? 1 : lg(x - 1) + 1;
}

/**
 * @brief LSB
 * @note
 * - 0を渡しちゃだめ
 */
constexpr std::size_t lsb(const std::size_t x)
{
    return __builtin_ffs(x) - 1;
}

/**
 * @brief x以上の最小2冪
 */
constexpr std::size_t ceil2(const std::size_t x)
{
    return 1UL << clg(x);
}

/**
 * @brief x以下の最大2冪
 */
constexpr std::size_t floor2(const std::size_t x)
{
    return 1UL << lg(x);
}
