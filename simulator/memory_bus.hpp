#pragma once
#include "data_cache.hpp"

class memory_bus
{
public:
    memory_bus(const std::size_t B, const std::size_t M) : m_cache{B, M} {}

    template<typename T>
    void write(disk_var<T>& dv, const T& val)
    {
        m_cache.insert_address_range(dv.addr(), sizeof(T), true);
        dv.m_val = val;
    }

    template<typename T>
    const T& read(const disk_var<T>& dv)
    {
        m_cache.insert_address_range(dv.addr(), sizeof(T), false);
        return dv.m_val;
    }

    statistic_info statistic();

private:
    data_cache m_cache;
};
