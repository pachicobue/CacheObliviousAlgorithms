#pragma once
#include <cstdint>
#include <limits>
using data_t            = int64_t;
constexpr data_t Min    = std::numeric_limits<data_t>::min() / 4;
constexpr data_t Max    = std::numeric_limits<data_t>::max() / 4;
constexpr uint64_t Seed = 20200810;
