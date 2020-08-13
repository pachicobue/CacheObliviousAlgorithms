#pragma once
#include "b_tree_node.hpp"
#include "data_cache.hpp"
#include "output_utility.hpp"
#include "safe_array.hpp"
/**
 * @brief 静的B-木
 * @details 
 * CacheAwareな探索木
 * LowerBoundのみをサポートし、Insert/Eraseなどは(構築時に利用はするが)サポートしない
 *
 * 各ノードは以下のフィールドを持つ
 * - keys   ：2K-1個のキー領域(K-1以上が有効なキー)
 * - sons   ：2K個の子ノード領域(＋以下の情報を表現)
 * - size   ：有効なキーの個数(K-1以上2K-1以下)
 *            陽には持たず、sonsの先頭 size+1 個を非NULLに設定することで表現
 * - is_leaf：葉であるかどうか
 *            陽には持たず、葉の場合はsonsの先頭 size+1 個を自分自身のアドレスに設定することで表現
 *
 * また探索木としての性質として以下が成立している
 * - keysは昇順
 * - sons[i]に含まれるキーの値はkeys[i-1]とkeys[i]の間の値となっている
 */
template<typename Key, std::size_t K, std::size_t B, std::size_t M, bool Caching = true>
class b_tree
{
    using key_t  = Key;
    using node_t = b_tree_node_t<Key, K>;

public:
    /**
     * @brief コンストラクタ
     */
    b_tree(const std::vector<key_t>& keys) : m_cache{}
    {
        static_assert(K > 1, "K is should be more than 1");
        std::vector<node_t> nodes;
        const std::size_t ri = b_tree_node_t<Key, K>::build(keys, nodes);
        const std::size_t N  = nodes.size();
        m_nodes              = safe_array<node_t, B>(N);
        for (std::size_t i = 0; i < N; i++) {
            for (std::size_t j = 0; j < MaxNodeKeyNum; j++) { m_nodes[i].keys[j] = nodes[i].keys[j]; }
            for (std::size_t j = 0; j <= MaxNodeKeyNum; j++) {
                const node_t* son = reinterpret_cast<node_t*>(nodes[i].sons[j]);
                if (son == nullptr) {
                    m_nodes[i].sons[j] = nodes[i].sons[j];
                } else {
                    const std::size_t ind = son - (&nodes[0]);
                    m_nodes[i].sons[j]    = reinterpret_cast<uintptr_t>(&m_nodes[ind]);
                }
            }
        }
        m_root = reinterpret_cast<uintptr_t>(&m_nodes[ri]);
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
        std::cout << "- nodes            : " << m_nodes << std::endl;
        std::cout << "- root             : " << hex_str(m_root) << std::endl;
        std::cout << "- tree             : " << std::endl;
        std::cout << "                     ";
        print_tree(m_root);
        std::cout << std::endl;
    }

    static constexpr std::size_t PageSize      = B;
    static constexpr std::size_t CacheSize     = M;
    static constexpr std::size_t MinNodeKeyNum = K - 1;
    static constexpr std::size_t MaxNodeKeyNum = 2 * K - 1;
    static constexpr std::size_t NodeSize      = sizeof(node_t);

private:
    void lower_bound(const uintptr_t addr, const key_t key, std::pair<bool, key_t>& min)
    {
        if (addr == reinterpret_cast<uintptr_t>(nullptr)) { return; }
        node_t node = m_cache.template disk_read<node_t>(addr);
        for (std::size_t i = 0; i < MaxNodeKeyNum; i++) {
            if (node.sons[i + 1] == reinterpret_cast<uintptr_t>(nullptr)) { break; }
            if (node.keys[i] >= key) {
                min = std::min(min, {false, node.keys[i]});
                if (node.keys[i] == key) { return; }
            }
        }
        if (node.sons[0] != addr) {
            std::size_t i = 0;
            for (; i < MaxNodeKeyNum; i++) {
                if (node.sons[i + 1] == reinterpret_cast<uintptr_t>(nullptr)) { break; }
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
        for (std::size_t i = 0; i < MaxNodeKeyNum; i++) {
            if (p->sons[i + 1] == reinterpret_cast<uintptr_t>(nullptr)) { break; }
            std::cout << p->keys[i] << ",";
        }
        std::cout << "]:";
        if (p->sons[0] != addr) {
            for (std::size_t i = 0; i <= MaxNodeKeyNum; i++) { print_tree(p->sons[i]); }
        }
        std::cout << ")";
    }
    safe_array<node_t, B> m_nodes;
    data_cache<B, M, Caching> m_cache;
    uintptr_t m_root = reinterpret_cast<uintptr_t>(nullptr);
};
