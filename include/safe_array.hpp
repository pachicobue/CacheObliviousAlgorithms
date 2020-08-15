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
    safe_array() : sz{0}, buffer(Margin * 2) {}
    safe_array(const std::size_t N, const Data& init = data_type{}) : sz{N}, buffer(sz + Margin * 2, init) {}
    safe_array(const std::vector<Data>& vs) : sz{vs.size()}, buffer(sz + Margin * 2)
    {
        for (std::size_t i = 0; i < sz; i++) { buffer[i + Margin] = vs[i]; }
    }
    Data& operator[](const std::size_t i) { return assert(i < sz), buffer[i + Margin]; }
    const Data& operator[](const std::size_t i) const { return assert(i < sz), buffer[i + Margin]; }
    void push_back(const Data& data)
    {
        buffer.push_back(data);
        sz++;
        std::swap(buffer[sz + 2 * Margin - 1], buffer[sz + Margin - 1]);
    }
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
    std::size_t sz                      = 0;
    std::vector<Data> buffer;
};
