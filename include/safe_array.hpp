#pragma once
#include <cassert>
#include <cstdint>
#include <vector>
/**
 * @brief ページアクセスを安全に行える可変長配列
 * @details
 * このシミュレータでは B byte 単位でアクセスを行うが、このときmallocした領域外にアクセスがはみ出てしまう場合がある。
 * データ領域の左右にB byte 以上の領域を確保しておく
 */
template<typename T>
class safe_array
{
public:
    safe_array(const std::size_t B, const std::size_t N, const T& init = T{}) : sz{N}, offset{(B + sizeof(T) - 1) / sizeof(T)}, buffer(N + offset + offset, init) {}
    T& operator[](const std::size_t i) { return assert(i < sz), buffer[i + offset]; }
    const T& operator[](const std::size_t i) const { return assert(i < sz), buffer[i + offset]; }
    std::size_t size() const { return sz; }

private:
    std::size_t sz;
    std::size_t offset;
    std::vector<T> buffer;
};
