#pragma once
/**
 * @file statistic_info.hpp
 * @brief キャッシュミス回数の統計情報
 */
#include <cstdint>
/**
 * @brief 統計情報
 * @details
 * - disk_read_count：ディスクに読み込んだ回数(キャッシュミス回数)
 * - disk_write_count：ディスクに書き込んだ回数(キャッシュミス回数)
 */
struct statistic_info
{
    uint64_t disk_read_count  = 0;
    uint64_t disk_write_count = 0;
};
