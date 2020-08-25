#include "gnuplot.hpp"

gp_stream::gp_stream() : fp{popen("gnuplot -geometry 960x960 -persist", "w")},
                         p_fb{new __gnu_cxx::stdio_filebuf<char>(fp, std::ios_base::out)},
                         os{static_cast<std::streambuf*>(p_fb)}
{
}

gp_stream::~gp_stream()
{
    std::flush(os);
    delete p_fb;
    pclose(fp);
}

gp_stream& operator<<(gp_stream& gs, std::ostream& (*func)(std::ostream&))
{
    gs.os << func;
    return gs;
}
