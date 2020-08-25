cmake_minimum_required(VERSION 3.15)
add_library(Simulator STATIC data_cache.cpp memory_bus.cpp simulator.cpp)

add_unittest(disk_variable_test)
add_unittest(memory_bus_test)
add_unittest(simulator_test)
