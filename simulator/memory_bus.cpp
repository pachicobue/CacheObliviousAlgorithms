#include "memory_bus.hpp"

statistic_info memory_bus::statistic()
{
    return m_cache.statistic();
}
