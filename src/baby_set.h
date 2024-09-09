//
// Created by DELL on 2024/8/22.
//

#include "rb_tree.h"

#ifndef CPPBABYSTL_BABY_SET_H
#define CPPBABYSTL_BABY_SET_H

template<typename K, typename Compare = std::less<K>>
class BMultiSet {
public:
    using RBTree = RBTree<K, K, RBTreeSetNode<K>, Compare>;
    using Iterator = typename RBTree::Iterator;
    using SizeType = std::size_t;

private:
    RBTree tree_;

// @{  // 各类构造函数 / 析构函数
public:

    // 默认构造函数
    BMultiSet() = default;

    // 拷贝构造函数
    BMultiSet(const BMultiSet &other) {
        for (const K &key : other.tree_) {
            tree_.insert(true, key);
        }
    }

    // 移动构造函数
    BMultiSet(BMultiSet &&other) noexcept
            : tree_(std::move(other.tree_)) {}

    // 通过初始化列表构造对象
    BMultiSet(std::initializer_list<K> init_list) {
        for (const auto &it : init_list) tree_.insert(true, it);
    }

    template<typename InputIter>
    BMultiSet(InputIter beg, InputIter end) {
        while (beg != end) {
            tree_.insert(true, *beg);
            ++beg;
        }
    }

    ~BMultiSet() { tree_.clear(); }
// @}  // 各类构造函数 / 析构函数


// @{  // 赋值运算符相关操作
public:

    // 拷贝赋值函数
    BMultiSet& operator=(const BMultiSet &other) {
        if (this == &other) return *this;

        tree_.clear();
        for (const auto &key : other.tree_)
            tree_.insert(true, key);
        return *this;
    }

    // 移动赋值函数
    BMultiSet& operator=(BMultiSet &&other) noexcept {
        tree_ = std::move(other.tree_);
        return *this;
    }

    // 通过初始化列表赋值
    BMultiSet& operator=(std::initializer_list<K> init_list) {
        tree_.clear();
        for (const auto &it : init_list) tree_.insert(true, it);
    }
// @}  // 赋值运算符相关操作


// @{  // 迭代器相关，直接复用 RBTree 的迭代器即可
public:

    Iterator begin() { return tree_.begin(); }
    Iterator begin() const { return tree_.begin(); }
    Iterator end() { return tree_.end(); }
    Iterator end() const { return tree_.end(); }
// @}  // 迭代器相关，直接复用 RBTree 的迭代器即可


// @{  // 与容量相关的操作
public:

    // 检查容器是否为空
    bool empty() const {
        return tree_.size() == 0;
    }

    // 返回容器中的元素数
    SizeType size() const {
        return tree_.size();
    }
// @}  // 与容量相关的操作


// @{  // 向容器中增加元素相关的操作
public:

    std::pair<Iterator, bool> insert(const K &key) {
        return {tree_.insert(true, key), true};
    }

    std::pair<Iterator, bool> insert(K &&key) {
        return {tree_.insert(true, key), true};
    }

    template <typename InputIter>
    void insert(InputIter beg, InputIter end) {
        while (beg != end) {
            tree_.insert(true, *beg);
            ++beg;
        }
    }

    void insert(std::initializer_list<K> init_list) {
        for (const auto &it : init_list)
            tree_.insert(true, it);
    }
// @}  // 向容器中增加元素相关的操作


// @{  // 向容器中删除元素的相关操作
public:

    void erase(const K &key) { tree_.remove(key); }
    void erase(K &&key) { tree_.remove(key); }

    void clear() { tree_.clear(); }
// @}  // 向容器中删除元素的相关操作


public:
    // 交换两个容器中的内容
    void swap(BMultiSet &other) noexcept {
        RBTree tmp = std::move(tree_);
        tree_ = std::move(other.tree_);
        other.tree_ = std::move(tmp);
    }


// @{  // 与元素查找相关的操作
public:

    // 返回键为`key`的元素数。由于BSet容器不允许存在相同的键，因此 count() 的
    // 返回值非 0 即 1。其中 0 表示不存在键`key`，1 表示存在键`key`
    SizeType count(const K &key) const {
        return tree_.find(key) != tree_.end();
    }

    // 寻找键为`key`的元素
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
// @}  // 与元素查找相关的操作
};



template<typename K, typename Compare>
bool operator==(const BMultiSet<K, Compare> &x, const BMultiSet<K, Compare> &y) {
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

#endif //CPPBABYSTL_BABY_SET_H
