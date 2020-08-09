#pragma once
#include "data_cache.hpp"
#include "output_utility.hpp"
#include "safe_array.hpp"
/**
 * @brief 静的B-木
 * @details 
 * CacheAwareな探索木
 * LowerBoundのみをサポートし、Insert/Eraseなどはサポートしない
 *
 * 各ノードは以下のフィールドを持つ
 * - keys   ：K個のキー
 * - sons   ：K+1個の子ノード(葉の場合は意味のない値が入る)
 * - is_leaf：葉かどうか
 *
 * また探索木としての性質として以下が成立している
 * - keysは昇順
 * - sons[i]に含まれるキーの値はkeys[i-1]とkeys[i]の間の値となっている
 */
template<typename Key, std::size_t B, std::size_t M, std::size_t K>
class b_tree
{
    using key_t = Key;
    struct node_t
    {
        node_t() = default;
        key_t keys[K];
        uintptr_t sons[K + 1];
        bool is_leaf = true;
        std::size_t sz;
        friend std::ostream& operator<<(std::ostream& os, const node_t& node)
        {
            os << "{leaf=" << node.is_leaf << ", sz=" << node.sz << ", keys=[";
            for (std::size_t i = 0; i < node.sz; i++) { os << node.keys[i] << ","; }
            os << "]";
            if (not node.is_leaf) {
                os << ", sons=[";
                for (std::size_t i = 0; i < NodeKeyNum + 1; i++) { os << hex_str(node.sons[i]) << ","; }
                os << "]";
            }
            return (os << "}");
        }
    };

public:
    /**
     * @brief コンストラクタ
     */
    b_tree(std::vector<key_t> keys) : m_nodes((keys.size() * (K + 2) + 2 * K) / (2 * K + 1)), m_cache{}
    {
        std::sort(keys.begin(), keys.end());
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
        m_root = build(keys, 0, keys.size());
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
     * @brief 統計情報
     */
    statistic_info statistic() const { return m_cache.statistic(); }
    /**
     * @brief 統計情報などの出力
     */
    void print_summary() const
    {
        std::cout << "[B-Tree Property]" << std::endl;
        std::cout << "- key num   (K): " << NodeKeyNum << std::endl;
        std::cout << "- node size (M): " << NodeSize << " byte" << std::endl;
        m_cache.print_summary();
    }
    /**
     * @brief デバッグ出力
     */
    void debug_print() const
    {
        m_cache.debug_print();
        std::cout << "[B-Tree Property]" << std::endl;
        std::cout << "- key num   (K): " << NodeKeyNum << std::endl;
        std::cout << "- node size (M): " << NodeSize << " byte" << std::endl;
        std::cout << "[Internal Status]" << std::endl;
        std::cout << "- index        : " << m_index << std::endl;
        std::cout << "- nodes        : " << m_nodes << std::endl;
        std::cout << "- root         : " << hex_str(m_root) << std::endl;
        std::cout << "- tree         : " << std::endl;
        std::cout << "                 ";
        print_tree(m_root);
        std::cout << std::endl;
    }

    static constexpr std::size_t PageSize   = B;
    static constexpr std::size_t CacheSize  = M;
    static constexpr std::size_t NodeKeyNum = K;
    static constexpr std::size_t NodeSize   = sizeof(node_t);

private:
    node_t* alloc()
    {
        assert(m_index < m_nodes.size());
        return &m_nodes[m_index++];
    }
    uintptr_t build(const std::vector<key_t>& keys, const std::size_t first, const std::size_t last)
    {
        const std::size_t sz = last - first;
        if (sz == 0) { return reinterpret_cast<uintptr_t>(nullptr); }
        if (sz <= NodeKeyNum) {
            auto* ptr    = alloc();
            ptr->is_leaf = true;
            ptr->sz      = sz;
            for (std::size_t i = 0; i < sz; i++) { ptr->keys[i] = keys[i + first]; }
            return reinterpret_cast<uintptr_t>(ptr);
        } else {
            const std::size_t subsz = (sz - NodeKeyNum) / (NodeKeyNum + 1);
            std::vector<std::size_t> szs(NodeKeyNum + 1, subsz);
            for (std::size_t i = 0; i < (sz - NodeKeyNum) % (NodeKeyNum + 1); i++) { szs[i]++; }
            auto* ptr         = alloc();
            ptr->is_leaf      = false;
            ptr->sz           = NodeKeyNum;
            std::size_t start = first;
            for (std::size_t i = 0; i < NodeKeyNum; i++) {
                ptr->keys[i] = keys[start + szs[i]];
                ptr->sons[i] = build(keys, start, start + szs[i]);
                start += szs[i] + 1;
            }
            ptr->sons[NodeKeyNum] = build(keys, start, last);
            return reinterpret_cast<uintptr_t>(ptr);
        }
    }
    void lower_bound(const uintptr_t addr, const key_t key, std::pair<bool, key_t>& min)
    {
        if (addr == reinterpret_cast<uintptr_t>(nullptr)) { return; }
        node_t node = m_cache.template disk_read<node_t>(addr);
        for (std::size_t i = 0; i < node.sz; i++) {
            if (node.keys[i] >= key) {
                min = std::min(min, {false, node.keys[i]});
                if (node.keys[i] == key) { return; }
            }
        }
        if (not node.is_leaf) {
            std::size_t i = 0;
            for (; i < NodeKeyNum; i++) {
                if (node.keys[i] > key) { break; }
            }
            lower_bound(node.sons[i], key, min);
        }
    }
    static void print_tree(const uintptr_t addr)
    {
        if (addr == reinterpret_cast<uintptr_t>(nullptr)) {
            std::cout << "()";
            return;
        }
        const node_t* p = reinterpret_cast<const node_t*>(addr);
        std::cout << "([";
        for (std::size_t i = 0; i < p->sz; i++) { std::cout << p->keys[i] << ","; }
        std::cout << "]:";
        if (not p->is_leaf) {
            for (std::size_t i = 0; i <= NodeKeyNum; i++) { print_tree(p->sons[i]); }
        }
        std::cout << ")";
    }
    std::size_t m_index = 0;
    safe_array<node_t, B> m_nodes;
    data_cache<B, M> m_cache;
    uintptr_t m_root;
};
