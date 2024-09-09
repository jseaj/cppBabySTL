//
// Created by DELL on 2024/8/23.
//

#include "rb_tree.h"

#ifndef CPPBABYSTL_BABY_MULTIMAP_H
#define CPPBABYSTL_BABY_MULTIMAP_H

template<typename K, typename V,
         typename Compare = std::less<K>>
class BMultiMap {
public:
    using RBTree = RBTree<K, V, RBTreeMapNode<K, V>>;
    using Iterator = typename RBTree::Iterator;
    using size_type = std::size_t;

private:
    RBTree tree_;


// @{  // 各类构造函数 / 析构函数
public:

    // 默认构造函数
    BMultiMap() = default;

    // 拷贝构造函数
    BMultiMap(const BMultiMap &other) {
        for (const auto [k, v] : other.tree_)
            tree_.insert(false, k, v);
    }

    // 移动构造函数
    BMultiMap(BMultiMap &&other) noexcept
            : tree_(std::move(other.tree_)) {}

    // 通过初始化列表构造对象
    BMultiMap(std::initializer_list<std::pair<K, V>> init_list) {
        for (const auto &[k, v] : init_list)
            tree_.insert(false, k, v);
    }

    template<typename InputIter>
    BMultiMap(InputIter beg, InputIter end) {
        while (beg != end) {
            auto &[k, v] = *beg;
            tree_.insert(false, k, v);
            ++beg;
        }
    }

    ~BMultiMap() { tree_.clear(); }
// @}  // 各类构造函数 / 析构函数


// @{  // 赋值运算相关操作
public:

    // 拷贝赋值运算操作
    BMultiMap& operator=(const BMultiMap &other) {
        if (this == &other) return *this;

        tree_.clear();
        for (const auto [k, v] : other.tree_) {
            tree_.insert(false, k, v);
        }
        return *this;
    }

    // 移动赋值运算操作
    BMultiMap& operator=(BMultiMap &&other) noexcept {
        tree_ = std::move(other.tree_);
        return *this;
    }

    // 通过初始化列表进行赋值
    BMultiMap& operator=(
            std::initializer_list<std::pair<K, V>> init_list) {
        tree_.clear();
        for (const auto &[k, v] : init_list)
            tree_.insert(false, k, v);
        return *this;
    }
// @}  // 赋值运算相关操作


// @{  // 迭代器相关操作
public:

    Iterator begin() {return tree_.begin(); }
    Iterator begin() const {return tree_.begin(); }
    Iterator end() { return tree_.end(); }
    Iterator end() const { return tree_.end(); }
// @}  // 迭代器相关操作


// @{ 与容器容量相关的操作
public:

    bool empty() const {
        return tree_.size() == 0;
    }

    size_type size() const { return tree_.size(); }
// @} 与容器容量相关的操作


// @{  // 向容器中增加元素的相关操作
public:

    // 向容器中插入元素
    Iterator insert(const std::pair<K, V> &val) {
        auto &[k, v] = val;
        return tree_.insert(false, k, v);
    }

    Iterator insert(std::pair<K, V> &&val) {
        auto &[k, v] = val;
        return tree_.insert(false, k, v);
    }

    // 向容器中插入一个迭代器范围的元素
    template<typename InputIter>
    void insert(InputIter beg, InputIter end) {
        while (beg != end) {
            auto &[k, v] = *beg;
            tree_.insert(false, k, v);
            ++beg;
        }
    }

    // 向容器中插入一个初始化列表内的所有元素
    void insert(std::initializer_list<std::pair<K, V>> init_list) {
        for (const auto &[k, v] : init_list) {
            tree_.insert(false, k, v);
        }
    }

    // 向容器中原位构造元素
    template<typename ...Args>
    Iterator emplace(const K &key, Args ...args) {
        return tree_.insert(false, key, std::forward<Args>(args)...);
    }
// @}  // 向容器中增加元素的相关操作


// @{  // 在容器中删除元素的操作
public:

    // 在容器中删除所有键为`key`的元素，并返回已删除元素的数目
    size_type erase(const K &key) {
        size_type cnt = 0;
        while (tree_.remove(key)) cnt++;
        return cnt;
    }

    size_type erase(K &&key) { return this->erase(key); }
// @}  // 在容器中删除元素的操作


public:
    // 交换两个BMultiMap容器中的内容
    void swap(BMultiMap &other) noexcept {
        RBTree tmp = std::move(tree_);
        tree_ = std::move(other.tree_);
        other.tree_ = std::move(tmp);
    }

// @{  // 与元素查找相关的操作
public:

    // 返回键为`key`的元素数目
    size_type count(const K &key) const {
        Iterator beg = tree_.lower_bound(key);

        // 容器中不存在键为`key`的元素
        if (beg == tree_.end() || (*beg).first != key)
            return 0;

        Iterator end = tree_.upper_bound(key);

        size_type cnt = 0;
        while (beg != end) {
            ++cnt;
            ++beg;
        }
        return cnt + 1;
    }

    // 返回指向键为`key`的元素的迭代器。如果键为`key`的元素由多个，则可能返回
    // 其中的任何一个；如果不存在键为`key`的元素，则返回 end()
    Iterator find(const K &key) { return tree_.find(key); }
    Iterator find(const K &key) const { return tree_.find(key); }

    // 返回首个不小于（>=）`key`的元素的迭代器
    Iterator lower_bound(const K &key) {
        return tree_.lower_bound(key);
    }

    Iterator lower_bound(const K &key) const {
        return tree_.lower_bound(key);
    }

    // 返回首个大于`key`的元素的迭代器
    Iterator upper_bound(const K &key) {
        return tree_.upper_bound(key);
    }

    Iterator upper_bound(const K &key) const {
        return tree_.upper_bound(key);
    }
// @}  // 与元素查找相关的操作
};


template <typename K, typename V, typename Compare>
bool operator==(
        const BMultiMap<K, V, Compare> &x, const BMultiMap<K, V, Compare> &y) {
    if (x.size() != y.size()) return false;

    auto beg1 = x.begin();
    auto beg2 = y.begin();
    while (beg1 != x.end() && beg2 != y.end()) {
        // TODO: 该容器相等的判断条件没有这么简单
        if (*beg1 != *beg2) return false;
        ++beg1;
        ++beg2;
    }
    return true;
}

#endif //CPPBABYSTL_BABY_MULTIMAP_H
