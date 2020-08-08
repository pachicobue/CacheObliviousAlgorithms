cmake_minimum_required(VERSION 3.10)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -Wshadow")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fsanitize=undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
project(CacheObliviousAlgorithms CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories("include")


add_executable(main main.cpp src/data_cache.cpp)

add_executable(summation example/summation.cpp src/data_cache.cpp)
add_executable(median example/median.cpp src/data_cache.cpp)


add_subdirectory(thirdparty/googletest/googletest)

add_executable(safe_array_test test/safe_array_test.cpp)
target_link_libraries(safe_array_test gtest_main pthread)
target_include_directories(safe_array_test PUBLIC thirdparty/gtest/googletest/include)  

add_executable(data_cache_test test/data_cache_test.cpp src/data_cache.cpp)
target_link_libraries(data_cache_test gtest_main pthread)
target_include_directories(data_cache_test PUBLIC thirdparty/gtest/googletest/include)  
