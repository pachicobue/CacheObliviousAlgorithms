#pragma once
#include <cstdint>
#include <set>
#include <vector>

/**
 * @brief DCacheの仮想化
 * @details
 *   パラメータ B = 一回に読む要素数
 *   パラメータ M = キャッシュ要素数
 */
class data_cache
{
public:
    /**
     * @brief コンストラクタ
     * @param B[in] 一回に読む要素数
     * @param M[in] キャッシュ要素数
     */
    data_cache(const std::size_t B, const std::size_t M);

    /**
     * @brief 指定アドレスがキャッシュに存在するか
     * @param addr[in] アドレス
     */
    bool contain(const std::uintptr_t addr) const;

    /**
     * @brief アドレスブロックを挿入する
     * @param addr_block[in] アドレスブロック
     * @details LURでの追い出し＋TSの更新も行う
     */
    void insert(const std::vector<std::uintptr_t>& addr_block);

    /**
     * @brief 指定アドレスをキャッシュから消す
     * @param addr[in] アドレス
     */
    void erase(const std::uintptr_t addr);

    /**
     * @brief ブロックサイズ B
     */
    std::size_t block_size() const;

    /**
     * @brief キャッシュサイズ M
     */
    std::size_t cache_size() const;

private:
    const std::size_t m_block_size, m_cache_size;
    std::uint64_t m_time;
    std::set<std::pair<std::uintptr_t, uint64_t>> m_items;
    std::set<std::pair<uint64_t, std::uintptr_t>> m_metis;
};
