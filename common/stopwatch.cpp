#include "stopwatch.hpp"

stopwatch::stopwatch() : m_start{std::chrono::system_clock::now()},
                         m_rap_point{m_start} {}
