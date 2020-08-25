#pragma once
/**
 * @file gnuplot.hpp
 * @brief gnuplotをC++から使うためのやつ
 */
#include <cstdlib>
#include <ext/stdio_filebuf.h>
#include <ostream>

/**
 * @brief gnuplotストリーム
 * @note
 * - gp_out << "plot sin(x)" << std::endl; みたいに使う
 */
class gp_stream
{
public:
    /**
     * @brief コンストラクタ
     */
    gp_stream();

    /**
     * @brief デストラクタ
     */
    ~gp_stream();

    /**
     * @brief マニピュレータ適用
     * @param func[in] マニピュレータ
     */
    friend gp_stream& operator<<(gp_stream& gs, std::ostream& (*func)(std::ostream&));

    /**
     * @brief ストリーム出力
     * @param arg[in] 任意の値
     */
    template<typename Arg>
    friend gp_stream& operator<<(gp_stream& gs, const Arg arg)
    {
        gs.os << arg;
        return gs;
    }

private:
    FILE* fp;
    __gnu_cxx::stdio_filebuf<char>* p_fb;
    std::ostream os;
};
