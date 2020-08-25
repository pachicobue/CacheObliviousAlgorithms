#pragma once
#include <cstdint>
using data_t            = int64_t;
constexpr data_t Min    = -(static_cast<data_t>(1) << 20);
constexpr data_t Max    = static_cast<data_t>(1) << 20;
constexpr uint64_t Seed = 20200810;
