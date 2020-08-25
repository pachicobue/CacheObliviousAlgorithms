#pragma once
/**
 * @file rng.hpp
 * @brief 乱数生成器
 */
#include <limits>
#include <random>

/**
 * @brief 乱数生成器
 */
class rng_base
{
public:
    /**
     * @brief コンストラクタ
     * @param seed[in] シード値
     */
    rng_base(const uint64_t seed);

    /**
     * @brief 整数値乱数
     * @param min[in] 最小値
     * @param max[in] 最大値
     */
    template<typename T>
    T val(const T min, const T max)
    {
        return min + static_cast<T>(next(max - min));
    }

    /**
     * @brief 整数値random vector
     * @param n[in] 要素数
     * @param min[in] 最小値
     * @param max[in] 最大値
     */
    template<typename T>
    std::vector<T> vec(const int sz, const T min, const T max)
    {
        std::vector<T> vs(sz);
        for (auto& v : vs) {
            v = val<T>(min, max);
        }
        return vs;
    }

private:
    uint64_t next(const uint64_t max = std::numeric_limits<uint64_t>::max());

    std::mt19937_64 rng;
};
