#include <gtest/gtest.h>

#include "common/gnuplot.hpp"

TEST(GnuplotTest, Constructor)
{
    gp_stream gin;
}

TEST(GnuplotTest, Plot)
{
    gp_stream gin;
    gin << "plot sin(x)" << std::endl;
}

TEST(GnuplotTest, PlotInteractive)
{
    gp_stream gin;
    gin << "plot \'-\'" << std::endl;
    for (int i = 0; i < 100; i++) {
        gin << i << " " << i * i << std::endl;
    }
    gin << "e" << std::endl;
}
