#pragma once
#include <cstdlib>
#include <ext/stdio_filebuf.h>
#include <ostream>
class gp_stream
{
public:
    gp_stream() : fp{popen("gnuplot -geometry 960x960 -persist", "w")}, p_fb{new __gnu_cxx::stdio_filebuf<char>(fp, std::ios_base::out)}, os{static_cast<std::streambuf*>(p_fb)} {}

    ~gp_stream()
    {
        std::flush(os);
        delete p_fb;
        pclose(fp);
    }

    template<typename Arg>
    friend gp_stream& operator<<(gp_stream& gs, const Arg arg)
    {
        gs.os << arg;
        return gs;
    }

    void flush()
    {
        std::flush(os);
    }

private:
    FILE* fp;
    __gnu_cxx::stdio_filebuf<char>* p_fb;
    std::ostream os;
};
