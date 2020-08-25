#pragma once
/**
 * @file stopwatch.hpp
 * @brief 時間計測
 */
#include <chrono>
/**
 * @brief chronoのラッパー
 */
class stopwatch
{
public:
    stopwatch();
    template<typename Dur = std::chrono::milliseconds>
    long long rap()
    {
        const auto now = std::chrono::system_clock::now();
        const auto dur = std::chrono::duration_cast<Dur>(now - m_rap_point).count();
        m_rap_point    = now;
        return dur;
    }

private:
    std::chrono::system_clock::time_point m_start;
    std::chrono::system_clock::time_point m_rap_point;
};
