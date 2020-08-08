#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
/**
 * @brief ページアクセスを安全に行える可変長配列
 * @details
 * このシミュレータでは B byte 単位でアクセスを行う。
 * このときmallocした領域外にアクセスがはみ出てしまう場合がある。
 * N byte の領域が欲しい場合に N+2B-2 byte を裏で確保しておいて、Bの倍数の場所を先頭とする N byte 配列であるように振る舞う。
 * 
 * align制約のWarning的なのが出るのでBは8の倍数であることを要求する。
 */
template<typename T>
class safe_array
{
public:
    safe_array(const std::size_t B, const std::size_t N) : sz{N}, buffer{reinterpret_cast<std::byte*>(malloc(N * sizeof(T) + B + B - 2))}, start{reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(buffer) + B - 1) / B * B)} { assert(B % 8 == 0); }
    T& operator[](const std::size_t i) { return assert(i < sz), start[i]; }
    const T& operator[](const std::size_t i) const { return assert(i < sz), start[i]; }
    std::size_t size() const { return sz; }

private:
    std::size_t sz;
    std::byte* buffer;
    T* start;
};
