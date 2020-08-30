#include "b_tree.hpp"
#include "simulator/simulator.hpp"

namespace {

using ptr_t = b_tree::ptr_t;

ptr_t alloc()
{
    return std::make_shared<b_tree::node_t>();
}

void split_child(ptr_t x, const std::size_t i, const std::size_t K_)
{
    ptr_t z = alloc();
    ptr_t y = x->sons[i].illegal_ref();
    assert(y->keys.size() == 2 * K_ - 1);
    z->leaf = y->leaf;
    for (std::size_t j = 0; j < K_ - 1; j++) {
        z->keys.push_back(y->keys[j + K_]);
    }
    if (not y->leaf.illegal_ref()) {
        for (std::size_t j = 0; j < K_; j++) {
            z->sons.push_back(y->sons[j + K_]);
        }
    }
    x->keys.insert(x->keys.begin() + i, y->keys[K_ - 1]);
    x->sons.insert(x->sons.begin() + i + 1, z);
    y->keys.resize(K_ - 1);
    y->sons.resize(K_);
}

void insert_nonfull(ptr_t x, const data_t k, const std::size_t K_)
{
    std::size_t i = 0;
    for (; i < x->keys.size(); i++) {
        if (k < x->keys[i].illegal_ref()) { break; }
    }
    if (x->leaf.illegal_ref()) {
        x->keys.insert(x->keys.begin() + i, k);
    } else {
        if (x->sons[i].illegal_ref()->keys.size() == 2 * K_ - 1) {
            split_child(x, i, K_);
            if (k >= x->keys[i].illegal_ref()) {
                i++;
            }
        }
        insert_nonfull(x->sons[i].illegal_ref(), k, K_);
    }
}

}  // anonymous namespace

b_tree::b_tree(const std::size_t K_) : K{K_}, m_root{alloc()}
{
    m_root->leaf = true;
}

b_tree::b_tree(const std::vector<data_t>& datas, const std::size_t K_) : K{K_}, m_root{alloc()}
{
    m_root->leaf = true;
    for (const auto data : datas) {
        illegal_insert(data);
    }
}

void b_tree::illegal_insert(const data_t key)
{
    if (m_root->keys.size() == 2 * K - 1) {
        auto r = m_root;
        auto s = alloc();
        s->sons.push_back(disk_var<ptr_t>{r});
        m_root = s;
        split_child(m_root, 0, K);
    }
    insert_nonfull(m_root, key, K);
}

data_t b_tree::lower_bound(const data_t key) const
{
    ptr_t p    = m_root;
    data_t max = Max + 1;
    while (true) {
        for (std::size_t i = 0; i < p->keys.size(); i++) {
            const data_t k = sim::read(p->keys[i]);
            if (key <= k) {
                max = std::min(max, k);
                if (k == key) { return k; }
            }
        }
        if (not sim::read(p->leaf)) {
            std::size_t i = 0;
            for (; i < p->keys.size(); i++) {
                const data_t k = sim::read(p->keys[i]);
                if (key < k) {
                    break;
                }
            }
            p = sim::read(p->sons[i]);
        } else {
            break;
        }
    }
    return max;
}
