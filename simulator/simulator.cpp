#include "simulator.hpp"
namespace sim {

memory_bus* g_bus_ptr = new memory_bus{1, 1};

void initialize(const std::size_t B, const std::size_t M)
{
    delete (g_bus_ptr);
    g_bus_ptr = new memory_bus{B, M};
}

statistic_info cache_miss_count()
{
    return g_bus_ptr->statistic();
}

}  // namespace sim
