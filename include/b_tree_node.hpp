#pragma once
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include "output_utility.hpp"
/**
 * @brief B-木の構築部分
 * @details そのうちb_tree.hppに組み込む予定
 */
template<typename Key, std::size_t K>
struct b_tree_node_t
{
    b_tree_node_t() = default;
    Key keys[2 * K - 1];
    uintptr_t sons[2 * K];
    friend std::ostream& operator<<(std::ostream& os, const b_tree_node_t& node)
    {
        os << "{keys=[";
        for (std::size_t i = 0; i < 2 * K - 1; i++) {
            if (node.sons[i + 1] == reinterpret_cast<uintptr_t>(nullptr)) { break; }
            os << node.keys[i] << ",";
        }
        os << "], sons=[";
        for (std::size_t i = 0; i < 2 * K; i++) {
            if (node.sons[i] == reinterpret_cast<uintptr_t>(nullptr)) { break; }
            os << hex_str(node.sons[i]) << ",";
        }
        os << "]";
        return (os << "}");
    }
    static std::size_t build(std::vector<Key> ks, std::vector<b_tree_node_t>& b_tree_nodes)
    {
        std::size_t ri = 0;
        struct node_t
        {
            node_t()       = default;
            std::size_t sz = 0;
            bool is_leaf   = false;
            Key keys[2 * K - 1];
            std::size_t sons[2 * K];
        };
        std::vector<node_t> nodes;

        auto alloc = [&]() -> std::size_t {
            node_t node{};
            for (std::size_t i = 0; i < 2 * K; i++) { node.sons[i] = reinterpret_cast<uintptr_t>(nullptr); }
            nodes.push_back(node);
            return nodes.size() - 1;
        };
        auto split_child = [&](std::size_t xi, const std::size_t ci) -> void {
            const std::size_t xn = nodes[xi].sz;
            const std::size_t zi = alloc();
            const std::size_t yi = nodes[xi].sons[ci];
            nodes[zi].is_leaf    = nodes[yi].is_leaf;
            nodes[zi].sz         = K - 1;
            for (std::size_t i = 0; i < K - 1; i++) { nodes[zi].keys[i] = nodes[yi].keys[i + K]; }
            if (not nodes[yi].is_leaf) {
                for (std::size_t i = 0; i < K; i++) { nodes[zi].sons[i] = nodes[yi].sons[i + K]; }
            }
            nodes[yi].sz = K - 1;
            for (std::size_t i = xn; i >= ci + 1; i--) { nodes[xi].sons[i + 1] = nodes[xi].sons[i]; }
            nodes[xi].sons[ci + 1] = zi;
            for (int i = (int)xn - 1; i >= (int)ci; i--) { nodes[xi].keys[i + 1] = nodes[xi].keys[i]; }
            nodes[xi].keys[ci] = nodes[yi].keys[K - 1];
            nodes[xi].sz++;
        };
        auto insert_non_full = [&](auto f, const std::size_t xi, const Key key) -> void {
            const std::size_t xn = nodes[xi].sz;
            if (nodes[xi].is_leaf) {
                std::size_t i = 0;
                for (; i < xn; i++) {
                    if (key < nodes[xi].keys[i]) { break; }
                }
                for (int j = (int)xn - 1; j >= (int)i; j--) { nodes[xi].keys[j + 1] = nodes[xi].keys[j]; }
                nodes[xi].keys[i] = key;
                nodes[xi].sz++;
            } else {
                std::size_t i = 0;
                for (; i < xn; i++) {
                    if (key < nodes[xi].keys[i]) { break; }
                }
                const std::size_t yi = nodes[xi].sons[i];
                if (nodes[yi].sz == 2 * K - 1) {
                    split_child(xi, i);
                    if (key > nodes[xi].keys[i]) { i++; }
                }
                f(f, nodes[xi].sons[i], key);
            }
        };
        auto insert = [&](const Key key) -> void {
            if (nodes[ri].sz == 2 * K - 1) {
                const std::size_t nri = alloc();
                nodes[nri].sons[0]    = ri;
                nodes[nri].sz         = 0;
                nodes[nri].is_leaf    = false;
                split_child(nri, 0);
                insert_non_full(insert_non_full, nri, key);
                ri = nri;
            } else {
                insert_non_full(insert_non_full, ri, key);
            }
        };

        ri                = alloc();
        nodes[ri].is_leaf = true;
        std::sort(ks.begin(), ks.end());
        ks.erase(std::unique(ks.begin(), ks.end()), ks.end());
        for (const key_t k : ks) { insert(k); }

        b_tree_nodes.resize(nodes.size());
        auto convert = [&](const node_t& node, b_tree_node_t<Key, K>& b_tree_node) -> void {
            const std::size_t sz = node.sz;
            for (std::size_t i = 0; i < sz; i++) { b_tree_node.keys[i] = node.keys[i]; }
            for (std::size_t i = 0; i <= sz; i++) { b_tree_node.sons[i] = reinterpret_cast<uintptr_t>(node.is_leaf ? &b_tree_node : &b_tree_nodes[node.sons[i]]); }
            for (std::size_t i = sz + 1; i < 2 * K; i++) { b_tree_node.sons[i] = reinterpret_cast<uintptr_t>(nullptr); }
        };
        for (std::size_t i = 0; i < nodes.size(); i++) { convert(nodes[i], b_tree_nodes[i]); }
        return ri;
    }
};
