#pragma once
/**
 * @file b_tree.hpp
 * @brief B-木
 */
#include "config.hpp"
#include "simulator/disk_variable.hpp"
/**
 * @brief B-木
 * @details Cache Awareなデータ構造
 * @note
 * 中間ノードは以下のデータを持つ (根のキー数はK-1未満でもOK ＋ 葉のsonsは空)
 * - keys：k個のキー (キー数kは K-1 <= k <= 2K-1)
 * - sons：k+1個の子ノード
 *
 * さらに探索木としての性質として以下が成立している
 * - keysは昇順
 * - sons[i]に含まれるキーは、keys[i-1]以上＆keys[i]未満
 */
class b_tree
{
    struct node_t
    {
        node_t() = default;
        std::vector<disk_var<data_t>> keys{};
        std::vector<disk_var<node_t*>> sons{};
        disk_var<bool> leaf{false};
    };

public:
    /**
     * @brief コンストラクタ
     * @param K[in] キー数に関する定数
     */
    b_tree(const std::size_t K_);

    /**
     * @brief コンストラクタ
     * @param K[in] キー数に関する定数
     * @param datas[in] 初期データ
     */
    b_tree(const std::size_t K_, const std::vector<data_t>& datas);

    /**
     * @brief 挿入
     * @param key[in] キー
     */
    void insert(const data_t key);

    /**
     * @brief LowerBound
     * @param key[in] キー
     */
    data_t lower_bound(const data_t key) const;

    using node_t = node_t;

private:
    void illegal_insert(const data_t key);
    const std::size_t K;
    node_t* m_root;
};
