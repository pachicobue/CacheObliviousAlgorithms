#pragma once
#include <cstdint>
#include <limits>
using data_t            = uint64_t;
constexpr data_t Max    = std::numeric_limits<data_t>::max() / 2;
constexpr data_t Min    = std::numeric_limits<data_t>::min() / 2;
constexpr uint64_t Seed = 20201013;
