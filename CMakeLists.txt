cmake_minimum_required(VERSION 3.15)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -Wshadow")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fsanitize=undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -mavx2")
project(CacheObliviousAlgorithms CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(".")

add_subdirectory(thirdparty/googletest/googletest)
enable_testing()

function(add_unittest test_name)
  set(args ${ARGV} "" "")	
  list(GET args 1 src_name)	
  add_executable(${test_name} test/${test_name}.cpp ${src_name})
  target_link_libraries(${test_name} gtest_main pthread Simulator Common)
  target_include_directories(${test_name} PUBLIC thirdparty/gtest/googletest/include)
  add_test(${test_name} ${test_name})
endfunction(add_unittest)

function(add_sim_example sim_example_name)
  add_executable(${sim_example_name} ${sim_example_name}.cpp)
  target_link_libraries(${sim_example_name} Simulator Common SimAlgorithm)
endfunction(add_sim_example)

function(add_actual_example actual_example_name)
  add_executable(${actual_example_name}_bench ${actual_example_name}.cpp)
  target_link_libraries(${actual_example_name}_bench Common)
endfunction(add_actual_example)

add_subdirectory(common)
add_subdirectory(simulator)
add_subdirectory(sim_algorithm)

add_subdirectory(sim_example)

add_subdirectory(actual_example)

