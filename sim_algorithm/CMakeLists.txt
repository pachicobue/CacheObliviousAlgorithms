cmake_minimum_required(VERSION 3.15)
add_library(SimAlgorithm STATIC vEB_search.cpp block_search.cpp binary_search.cpp)

add_unittest(vEB_search_test vEB_search.cpp)
add_unittest(block_search_test block_search.cpp)
add_unittest(binary_search_test binary_search.cpp)
