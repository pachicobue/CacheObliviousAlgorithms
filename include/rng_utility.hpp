#pragma once
#include <algorithm>
#include <limits>
#include <random>

template<typename Rng>
class rng_base
{
public:
    using result_type = typename Rng::result_type;
    static constexpr result_type min() { return Rng::min(); }
    static constexpr result_type max() { return Rng::max(); }
    rng_base() : rng_base(std::random_device{}()) {}
    rng_base(const std::random_device::result_type seed) : rng(seed) {}
    ~rng_base() = default;
    result_type val(const result_type max = std::numeric_limits<result_type>::max())
    {
        if (max == std::numeric_limits<result_type>::max()) { return rng(); }
        result_type mask = 1;
        for (; mask < max + 1; mask <<= 1) {}
        mask--;
        while (true) {
            const result_type ans = rng() & mask;
            if (ans <= max) { return ans; }
        }
    }
    template<typename T> T val(const T min, const T max) { return min + T(val(max - min)); }
    operator bool() { return val<bool>(0, 1); }
    template<typename T> std::pair<T, T> pair(const T min, const T max) { return std::minmax(val<T>(min, max), val<T>(min, max)); }
    template<typename T> std::vector<T> vec(const int n, const T min, const T max)
    {
        std::vector<T> vs(n);
        for (auto& v : vs) { v = val<T>(min, max); }
        return vs;
    }

private:
    Rng rng;
};
rng_base<std::mt19937> rng;
rng_base<std::mt19937_64> rng64;
