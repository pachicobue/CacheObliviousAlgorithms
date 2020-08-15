#pragma once
#include <random>

#include "data_cache.hpp"
#include "safe_array.hpp"
/**
 * @brief 静的B-木
 * @details 
 * CacheAwareな探索木(LowerBound/Insert クエリ)
 * Eraseはまだサポート外
 *
 * 各ノードは以下のフィールドを持つ
 * - keys   ：2K-1個のキー領域(K-1以上が有効なキー)
 * - sons   ：2K個の子ノード領域(＋以下の情報を表現)
 *
 * また探索木としての性質として以下が成立している
 * - keysは昇順
 * - sons[i]に含まれるキーの値はkeys[i-1]とkeys[i]の間の値となっている
 */
template<typename Key, std::size_t K, std::size_t B, std::size_t M, bool Caching = true>
class b_tree
{
    using key_t = Key;
    struct node_t
    {
        key_t keys[2 * K - 1];
        std::size_t sons[2 * K];
        static constexpr std::size_t nullind = 0xFFFFFFFF;
    };

public:
    /**
     * @brief コンストラクタ
     */
    b_tree() : m_cache{} { m_root = alloc(); }
    /**
     * @brief コンストラクタ
     * @param keys[in] キー集合
     */
    b_tree(std::vector<key_t> keys) : m_cache{}
    {
        m_root = alloc();
        std::sort(keys.begin(), keys.end());
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
        std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});
        for (const auto key : keys) { insert(key); }
        m_cache.reset();
    }
    /**
     * @brief 指定値以上の最小のKey
     * @param key[in] キー  
     * @returns [存在するか, キー]
     */
    std::pair<bool, key_t> lower_bound(const key_t key)
    {
        std::pair<bool, key_t> max{true, key};
        lower_bound(m_root, key, max);
        max.first = not max.first;
        return max;
    }
    /**
     * @brief 指定値を挿入
     * @param key[in] キー
     * @details キーに重複は許さない
     */
    void insert(const key_t key)
    {
        if (size_of(m_root) == 2 * K - 1) {
            const std::size_t nri = alloc();
            node_t nr_node        = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[nri]));
            nr_node.sons[0]       = m_root;
            m_root                = nri;
            m_cache.disk_write(reinterpret_cast<uintptr_t>(&m_nodes[m_root]), nr_node);
            split_child(nri, 0);
            insert_non_full(nri, key);
        } else {
            insert_non_full(m_root, key);
        }
    }
    /**
     * @brief ノード数
     */
    std::size_t node_num() const { return m_nodes.size(); }
    /**
     * @brief 統計情報
     */
    statistic_info statistic() const { return m_cache.statistic(); }
    /**
     * @brief 統計情報などの出力
     */
    void print_summary() const
    {
        std::cout << "[B-Tree Property]" << std::endl;
        std::cout << "- max key num(2K-1): " << MaxNodeKeyNum << std::endl;
        std::cout << "- min key num (K-1): " << MinNodeKeyNum << std::endl;
        std::cout << "- max node size    : " << NodeSize << " byte" << std::endl;
        std::cout << "- node num         : " << m_nodes.size() << std::endl;
        m_cache.print_summary();
    }
    /**
     * @brief デバッグ出力
     */
    void debug_print() const
    {
        m_cache.debug_print();
        std::cout << "[B-Tree Property]" << std::endl;
        std::cout << "- max key num(2K-1): " << MaxNodeKeyNum << std::endl;
        std::cout << "- min key num (K-1): " << MinNodeKeyNum << std::endl;
        std::cout << "- max node size    : " << NodeSize << " byte" << std::endl;
        std::cout << "- node num         : " << m_nodes.size() << std::endl;
        std::cout << "[Internal Status]" << std::endl;
        std::cout << "- root             : " << m_root << std::endl;
        std::cout << "- tree             : " << std::endl;
        print_subtree(m_root);
        std::cout << std::endl;
    }

    static constexpr std::size_t PageSize      = B;
    static constexpr std::size_t CacheSize     = M;
    static constexpr std::size_t MinNodeKeyNum = K - 1;
    static constexpr std::size_t MaxNodeKeyNum = 2 * K - 1;
    static constexpr std::size_t NodeSize      = sizeof(node_t);

private:
    std::size_t alloc()
    {
        node_t node;
        for (std::size_t i = 0; i < 2 * K; i++) { node.sons[i] = node_t::nullind; }
        node.sons[0] = m_nodes.size();
        m_nodes.push_back(node);
        m_cache.disk_write(reinterpret_cast<uintptr_t>(&m_nodes[m_nodes.size() - 1]), node);
        return m_nodes.size() - 1;
    }
    bool is_leaf(const std::size_t index)
    {
        const node_t node = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[index]));
        return node.sons[0] == index;
    }
    std::size_t size_of(const std::size_t index)
    {
        const node_t node = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[index]));
        std::size_t ans   = 0;
        for (; ans < 2 * K; ans++) {
            if (node.sons[ans] == node_t::nullind) { break; }
        }
        return ans - 1;
    }
    bool is_leaf(const std::size_t index) const { return m_nodes[index].sons[0] == index; }
    std::size_t size_of(const std::size_t index) const
    {
        std::size_t ans = 0;
        for (; ans < 2 * K; ans++) {
            if (m_nodes[index].sons[ans] == node_t::nullind) { break; }
        }
        return ans - 1;
    }
    void split_child(std::size_t xi, const std::size_t ci)
    {
        const std::size_t xn = size_of(xi);
        const std::size_t zi = alloc();
        node_t x_node        = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[xi]));
        const std::size_t yi = x_node.sons[ci];
        node_t y_node        = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[yi]));
        node_t z_node        = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[zi]));
        for (std::size_t i = 0; i < K - 1; i++) { z_node.keys[i] = y_node.keys[i + K]; }
        for (std::size_t i = 0; i < K; i++) { z_node.sons[i] = is_leaf(yi) ? zi : y_node.sons[i + K]; }
        for (std::size_t i = K; i < 2 * K; i++) { y_node.sons[i] = node_t::nullind; }
        for (std::size_t i = xn; i >= ci + 1; i--) { x_node.sons[i + 1] = x_node.sons[i]; }
        x_node.sons[ci + 1] = zi;
        for (int i = (int)xn - 1; i >= (int)ci; i--) { x_node.keys[i + 1] = x_node.keys[i]; }
        x_node.keys[ci] = y_node.keys[K - 1];
        m_cache.disk_write(reinterpret_cast<uintptr_t>(&m_nodes[xi]), x_node);
        m_cache.disk_write(reinterpret_cast<uintptr_t>(&m_nodes[yi]), y_node);
        m_cache.disk_write(reinterpret_cast<uintptr_t>(&m_nodes[zi]), z_node);
    };
    void insert_non_full(const std::size_t xi, const Key key)
    {
        const std::size_t xn = size_of(xi);
        node_t x_node        = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[xi]));
        if (is_leaf(xi)) {
            std::size_t i = 0;
            for (; i < xn; i++) {
                if (key < x_node.keys[i]) { break; }
            }
            for (int j = (int)xn - 1; j >= (int)i; j--) { x_node.keys[j + 1] = x_node.keys[j]; }
            for (std::size_t j = xn; j >= i + 1; j--) { x_node.sons[j + 1] = x_node.sons[j]; }
            x_node.keys[i]     = key;
            x_node.sons[i + 1] = xi;
            m_cache.disk_write(reinterpret_cast<uintptr_t>(&m_nodes[xi]), x_node);
        } else {
            std::size_t i = 0;
            for (; i < xn; i++) {
                if (key < x_node.keys[i]) { break; }
            }
            const std::size_t yi = x_node.sons[i];
            if (size_of(yi) == 2 * K - 1) {
                split_child(xi, i);
                x_node = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[xi]));
                if (key > x_node.keys[i]) { i++; }
            }
            insert_non_full(x_node.sons[i], key);
        }
    }
    void lower_bound(const std::size_t ind, const key_t key, std::pair<bool, key_t>& min)
    {
        const std::size_t sz = size_of(ind);
        const node_t node    = m_cache.template disk_read<node_t>(reinterpret_cast<uintptr_t>(&m_nodes[ind]));
        for (std::size_t i = 0; i < sz; i++) {
            if (node.keys[i] >= key) {
                min = std::min(min, {false, node.keys[i]});
                if (node.keys[i] == key) { return; }
            }
        }
        if (not is_leaf(ind)) {
            std::size_t i = 0;
            for (; i < sz; i++) {
                if (node.keys[i] > key) { break; }
            }
            lower_bound(node.sons[i], key, min);
        }
    }
    key_t min(std::size_t index) const { return is_leaf(index) ? m_nodes[index].keys[0] : min(m_nodes[index].sons[0]); }
    key_t max(std::size_t index) const
    {
        const std::size_t sz = size_of(index);
        return is_leaf(index) ? m_nodes[index].keys[sz - 1] : min(m_nodes[index].sons[sz]);
    }
    bool check(std::size_t index) const
    {
        const std::size_t sz = size_of(index);
        if (index != m_root and (sz < K - 1 or 2 * K - 1 < sz)) { return false; }

        for (std::size_t i = 0; i + 1 < sz; i++) {
            if (m_nodes[index].keys[i] >= m_nodes[index].keys[i + 1]) { return false; }
        }
        if (not is_leaf(index)) {
            for (std::size_t i = 0; i < sz; i++) {
                if (max(m_nodes[index].sons[i]) >= m_nodes[index].keys[i]) { return false; }
                if (min(m_nodes[index].sons[i + 1]) <= m_nodes[index].keys[i]) { return false; }
            }
            for (std::size_t i = 0; i <= sz; i++) {
                if (not check(m_nodes[index].sons[i])) { return false; }
            }
        }
        return true;
    }
    void print_subtree(std::size_t index) const
    {
        const std::size_t sz = size_of(index);
        std::cout << "                     [" << index << "]: keys=[";
        for (std::size_t i = 0; i < sz; i++) { std::cout << m_nodes[index].keys[i] << ","; }
        std::cout << "]";
        if (not is_leaf(index)) {
            std::cout << ", sons=[";
            for (std::size_t i = 0; i <= sz; i++) { std::cout << m_nodes[index].sons[i] << ","; }
            std::cout << "]";
        }
        std::cout << std::endl;
        if (not is_leaf(index)) {
            for (std::size_t i = 0; i <= sz; i++) { print_subtree(m_nodes[index].sons[i]); }
        }
    }
    safe_array<node_t, B> m_nodes;
    data_cache<B, M, Caching> m_cache;
    std::size_t m_root;
};
