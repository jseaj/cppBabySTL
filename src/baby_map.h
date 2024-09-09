//
// Created by DELL on 2024/8/19.
//

#include "rb_tree.h"

#ifndef CPPBABYSTL_BABY_MAP_H
#define CPPBABYSTL_BABY_MAP_H

template<typename K, typename V,
         typename Compare = std::less<K>>
class BMap {
public:
    using RBTree = RBTree<K, V, RBTreeMapNode<K, V>, Compare>;
    using Iterator = typename RBTree::Iterator;
    using SizeType = std::size_t;

private:
    RBTree tree_;

// @{  // 各类构造函数 / 析构函数
public:

    // 默认构造函数
    BMap() = default;

    // 拷贝构造函数
    BMap(const BMap &other) {
        for (const auto [k, v]: other.tree_) {
            tree_.insert(true, k, v);
        }
    }

    // 移动构造函数
    BMap(BMap &&other) noexcept
            : tree_(std::move(other.tree_)) {}

    // 通过初始化列表进行构造
    BMap(std::initializer_list<std::pair<K, V>> init_list) {
        for (const auto &[k, v]: init_list) {
            tree_.insert(true, k, v);
        }
    }

    template<typename InputIter>
    BMap(InputIter beg, InputIter end) {
        while (beg != end) {
            auto &[k, v] = *beg;
            tree_.insert(true, k, v);
            ++beg;
        }
    }

    ~BMap() { tree_.clear(); }
// @}  // 各类构造函数 / 析构函数


// @{ 与赋值运算相关的操作
public:

    // 重载拷贝赋值函数
    BMap& operator=(const BMap &other) {
        if (this == &other) return *this;  // 防止自赋值

        tree_.clear();
        for (auto [k, v] : other) {
            tree_.insert(true, k, v);
        }
    }

    // 重载移动赋值运函数
    BMap& operator=(BMap &&other) noexcept {
        tree_ = std::move(other.tree_);
        return *this;
    }

    // 将一个初始化列表赋值给当前容器
    BMap& operator=(std::initializer_list<std::pair<K, V>> init_list) {
        tree_.clear();
        for (const auto &[k, v] : init_list) {
            tree_.insert(true, k, v);
        }
        return *this;
    }
// @} // 与赋值运算相关的操作


// @{ // 元素访问相关的操作
public:

    V& at(const K &key) {
        auto iter = tree_.find(key);
        if (iter == tree_.end()) {
            throw std::out_of_range("BMap::at");
        }
        return (*iter).second;
    }

    const V& at(const K &key) const {
        return this->at(key);
    }

    V& operator[](const K &key) {
        auto iter = tree_.get_or_insert(key);
        return (*iter).second;
    }

    V& operator[](K &&key) {
        return this->operator[](key);
    }
// @} // 元素访问相关的操作


// @{  // 迭代器，直接复用红黑树的迭代器即可
public:
    // 迭代器相关，直接复用红黑树的迭代器即可

    Iterator begin() { return tree_.begin(); }
    Iterator begin() const { return tree_.begin(); }
    Iterator end() { return tree_.end(); }
    Iterator end() const { return tree_.end(); }
// @}  // 迭代器，直接复用红黑树的迭代器即可


// @{  // 容量相关的操作
public:

    // 检查容器是否为空
    bool empty() const noexcept {
        return tree_.size() == 0;
    }

    // 返回BMap容器的元素数目
    SizeType size() const noexcept {
        return tree_.size();
    }
// @}  // 容量相关的操作


// @{  // 向容器中添加元素相关的操作
public:
    // 与增加元素相关的操作

    std::pair<Iterator, bool>
    insert(const std::pair<K, V> &value) {
        auto &[k, v] = value;
        auto iter = tree_.insert(true, k, v);
        return {iter, true};
    }

    std::pair<Iterator, bool>
    insert(std::pair<K, V> &&value) {
        return this->insert(value);
    }

    template<typename InputIter>
    void insert(InputIter beg, InputIter end) {
        while (beg != end) {
            auto &[k, v] = *beg;
            tree_.insert(true, k, v);
            ++beg;
        }
    }

    void insert(std::initializer_list<std::pair<K, V>> init_list) {
        for (const auto &[k, v] : init_list) {
            tree_.insert(true, k, v);
        }
    }

    template<typename ...Args>
    std::pair<Iterator, bool>
    emplace(const K &key, Args &&...args) {
        auto iter = tree_.insert(true, key, std::forward<Args>(args)...);
        return {iter, true};
    }

    template<typename ...Args>
    std::pair<Iterator, bool>
    emplace(K &&key, Args &&...args) {
        return this->emplace(key, std::forward<Args>(args)...);
    }
// @}  // 向容器中添加元素相关的操作


// @{  // 在容器中删除元素相关的操作
public:

    void clear() noexcept {
        tree_.clear();
    }

    void erase(const K &key) { tree_.remove(key); }

    void erase(K &&key) { tree_.remove(key); }

    void swap(BMap &other) noexcept {
        RBTree tmp = std::move(tree_);
        tree_ = std::move(other.tree_);
        other.tree_ = std::move(tmp);
    }
// @}  // 在容器中删除元素相关的操作


// @{  // 与查找相关的操作
public:

    // 返回键为`key`的元素数。由于BMap容器不允许存在相同的键，因此 count() 的
    // 返回值非 0 即 1。其中 0 表示不存在键`key`，1 表示存在键`key`
    SizeType count(const K &key) {
        return tree_.find(key) != tree_.end();
    }

    // 返回指向键为`key`的迭代器，若不存在键为`key`的元素，返回 end()
    Iterator find(const K &key) {
        return tree_.find(key);
    }

    Iterator find(const K &key) const {
        return tree_.find(key);
    }

    // 返回指向首个不小于（>=）`key`的元素的迭代器
    Iterator lower_bound(const K &key) {
        return tree_.lower_bound(key);
    }

    Iterator lower_bound(const K &key) const {
        return tree_.lower_bound(key);
    }

    // 返回指向首个大于`key`的元素的迭代器
    Iterator upper_bound(const K &key) {
        return tree_.upper_bound(key);
    }

    Iterator upper_bound(const K &key) const {
        return tree_.upper_bound(key);
    }
// @}  // 与查找相关的操作
};

// 判断两个 BMap 容器是否相等
template<typename K, typename V, typename Compare>
bool operator==(const BMap<K, V, Compare> &x, const BMap<K, V, Compare> &y) {
    if (x.size() != y.size()) return false;

    auto beg1 = x.begin();
    auto beg2 = y.begin();
    while (beg1 != x.end() && beg2 != y.end()) {
        if (*beg1 != *beg2) return false;
        ++beg1;
        ++beg2;
    }
    return true;
}

#endif //CPPBABYSTL_BABY_MAP_H
