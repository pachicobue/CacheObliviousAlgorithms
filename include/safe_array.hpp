#pragma once
#include <cassert>
#include <iostream>
#include <vector>
/**
 * @brief ページアクセスを安全に行える可変長配列
 * @details
 * このシミュレータでは B byte 単位でアクセスを行うが、このときmallocした領域外にアクセスがはみ出てしまう場合がある。
 * データ領域の左右にB byte 以上の領域を確保しておく
 */
template<typename Data, std::size_t B>
class safe_array
{
public:
    using data_type = Data;
    safe_array(const std::size_t N, const Data& init = data_type{}) : sz{N}, buffer(N + Margin * 2, init) { buffer.shrink_to_fit(); }
    Data& operator[](const std::size_t i) { return assert(i < sz), buffer[i + Margin]; }
    const Data& operator[](const std::size_t i) const { return assert(i < sz), buffer[i + Margin]; }
    std::size_t size() const { return sz; }
    friend std::ostream& operator<<(std::ostream& os, const safe_array& vs)
    {
        os << "[";
        for (std::size_t i = 0; i < vs.size(); i++) { os << vs[i] << ","; }
        return (os << "]");
    }
    static constexpr std::size_t PageSize = B;

private:
    static constexpr std::size_t Margin = (PageSize + sizeof(data_type) - 1) / sizeof(data_type);
    std::size_t sz;
    std::vector<Data> buffer;
};
