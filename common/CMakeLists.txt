cmake_minimum_required(VERSION 3.15)
add_library(Common STATIC rng.cpp gnuplot.cpp stopwatch.cpp)
add_unittest(rng_test)
add_unittest(gnuplot_test)
