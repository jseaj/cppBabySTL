//
// Created by DELL on 2024/8/12.
//

#include <functional>
#include <cstddef>
#include <stdexcept>
#include <iostream>

#ifndef CPPBABYSTL_RB_TREE_H
#define CPPBABYSTL_RB_TREE_H

enum RBTreeColor {
    kRed = false,
    kBlack = true,
};

/*
 * 红黑树虚拟头节点
 *
 * 当红黑树不为 nullptr 时，parent 指向红黑树的 root 节点，此时
 * left 指向红黑树中序遍历的第一个节点，right 指向红黑树中序遍历的
 * 最后一个节点；
 * 当红黑树为 nullptr 时，parent = nullptr，同时 left 和 right
 * 均指向该节点本身。
 * */
struct NodeBase {
    NodeBase *parent;
    NodeBase *left, *right;

    NodeBase()
            : parent(nullptr),
              left(this), right(this) {}

    inline NodeBase *get_parent(const NodeBase *header) {
        if (this->parent == header) return nullptr;
        return this->parent;
    }

    inline NodeBase *get_uncle(const NodeBase *header) {
        NodeBase *p = get_parent(header);
        if (p == nullptr) return nullptr;

        NodeBase *grandpa = p->get_parent(header);
        if (grandpa == nullptr) return nullptr;

        if (grandpa->left == p) return grandpa->right;
        return grandpa->right;
    }

    inline NodeBase *get_sibling(const NodeBase *header) {
        NodeBase *p = get_parent(header);
        if (p == nullptr) return nullptr;

        if (p->left == this) return p->right;
        return p->left;
    }
};

/*
 * 红黑树节点，用于set
 *
 * 不同于NodeBase，默认插入的节点中其左右子树均为空，即
 * left = right = nullptr，同时节点颜色为`红`
 * */
template<typename Key>
struct RBTreeSetNode : public NodeBase {
    RBTreeColor color;  // 当前节点的颜色，仅有`红`或`黑`两种
    Key key;            // 当前节点存储的`键`值

    using ReturnType = const Key &;

    RBTreeSetNode() : NodeBase(), color(RBTreeColor::kRed) {
        this->left = this->right = nullptr;
    }

    explicit RBTreeSetNode(const Key &k)
            : NodeBase(),
              color(RBTreeColor::kRed),
              key(k) {
        this->left = this->right = nullptr;
    }

    template<typename ...Args>
    static RBTreeSetNode *create_node(const Key &k, Args &&... args) {
        return new RBTreeSetNode(k);
    }

    static void destroy_node(RBTreeSetNode *node, NodeBase *header) {
        auto p = node->get_parent(header);
        if (p != nullptr) {
            if (p->left == node) p->left = nullptr;
            if (p->right == node) p->right = nullptr;
        }
        node->key.~Key();
        delete node;
    }

    template<typename ...Args>
    void replace_val(Args &&...args) {}

    inline void swap(RBTreeSetNode *other) {
        std::swap(this->key, other->key);
    }

    ReturnType get_data() { return key; }
    ReturnType get_data() const { return key; }
};

/*
 * 红黑树节点，用于map
 *
 * 不同于NodeBase，默认插入的节点中其左右子树均为空，即
 * left = right = nullptr，同时节点颜色为`红`
 * */
template<typename Key, typename Value>
struct RBTreeMapNode : public NodeBase {
    RBTreeColor color;  // 当前节点的颜色，仅有`红`或`黑`两种
    Key key;            // 当前节点存储的`键`值
    Value val{};        // 当前节点存储的`值`

    using ReturnType = std::pair<const Key &, Value &>;

    RBTreeMapNode() : NodeBase(), color(RBTreeColor::kRed) {
        this->left = this->right = nullptr;
    }

    explicit RBTreeMapNode(const Key &k)
            : NodeBase(),
              color(RBTreeColor::kRed),
              key(k) {
        this->left = this->right = nullptr;
    }

    template<typename ...Args>
    static RBTreeMapNode *create_node(const Key &k, Args &&... args) {
        auto node = new RBTreeMapNode(k);
        new(&node->val) Value(std::forward<Args>(args)...);
        return node;
    }

    static void destroy_node(RBTreeMapNode *node, NodeBase *header) {
        auto p = node->get_parent(header);
        if (p != nullptr) {
            if (p->left == node) p->left = nullptr;
            if (p->right == node) p->right = nullptr;
        }
        node->key.~Key();
        node->val.~Value();
        delete node;
    }

    template<typename ...Args>
    void replace_val(Args &&...args) {
        val.~Value();
        new (&val) Value(std::forward<Args>(args)...);
    }

    inline void swap(RBTreeMapNode *other) {
        std::swap(this->key, other->key);
        std::swap(this->val, other->val);
    }

    ReturnType get_data() { return {key, val}; }
    ReturnType get_data() const { return {key, val}; }
};

/*
 * 红黑树
 *
 * @tparam K `键`的类型
 * @tparam V 红黑树节点真实存储的数据类型
 * @tparam Compare 可调用对象，用于 K 类型之间的比较
 * */
template<typename K, typename V, typename NodeType,
        typename Compare = std::less<K>>
class RBTree {
public:
    using SizeType = std::size_t;
    using ReturnType = typename NodeType::ReturnType;
    struct Iterator;

private:
    NodeBase header_;  // 红黑树root节点的头结点
    SizeType cnt_{};   // 红黑树节点的数目（不包括header_）
    Compare cmp_;      // 红黑树中用于比较节点内元素大小的函数

public:
    RBTree() = default;

    // 移动构造函数
    RBTree(RBTree &&other) noexcept;

    // 移动赋值函数
    RBTree &operator=(RBTree &&other) noexcept;

    ~RBTree() { this->clear(); }


// @{  // 在红黑树中插入元素相关的操作
private:

    /*
     * @brief 向红黑树中插入一个新节点
     * @param node 当前遍历到的红黑树节点
     * @param key 新插入节点的`键`
     * @param args 新插入节点的`值`
     * @param replace 是否替换掉已经存在的值
     * */
    template<typename ...Args>
    NodeType *insert_(NodeType *node, bool replace,
                      const K &key, Args &&... args);

    // 左旋操作
    void rotate_left_(NodeBase *node);

    // 右旋操作
    void rotate_right_(NodeBase *node);

    // 插入节点node后进行的调整平衡的操作
    void rebalance_after_insert_(NodeType *node);

public:

    template<typename ...Args>
    Iterator insert(bool replace, const K &key, Args &&... args);

    template<typename ...Args>
    Iterator insert(bool replace, K &&key, Args &&... args) {
        return this->insert(replace, key, std::forward<Args>(args)...);
    }

    Iterator get_or_insert(const K &key) {
        if (cnt_ == 0) return Iterator(&header_, &header_);

        auto root = static_cast<NodeType*>(header_.parent);
        auto node = find_(root, key);
        if (node == nullptr) {
            node = insert_(root, true, key);
        }
        return Iterator(node, &header_);
    }
// @}  // 在红黑树中插入元素相关的操作


// @{  // 在红黑树中删除元素的相关操作
private:

    // 删除掉节点 node
    void destroy_node_(NodeType *node);

    // 清除掉以node为根的树
    void clear_(NodeBase *node);

    // 删除一个黑色叶子节点 node 后，对红黑树的调整操作
    void rebalance_after_remove_(NodeType *node);

    //在以 node 为根节点的树中删除`键`为 key 的节点
    bool remove_(NodeType *node, const K &key);

public:

    bool remove(K &&key) {
        auto root = static_cast<NodeType *>(header_.parent);
        return remove_(root, key);
    }

    bool remove(const K &key) {
        auto root = static_cast<NodeType *>(header_.parent);
        return remove_(root, key);
    }

    void clear() {
        this->clear_(header_.parent);
        header_.parent = nullptr;
        header_.left = header_.right = &header_;
        cnt_ = 0;
    }
// @}  // 在红黑树中删除元素的相关操作


// @{  // 红黑树的迭代器
public:

    Iterator begin() {
        return Iterator(header_.left, &header_);
    }

    Iterator begin() const {
        return Iterator(header_.left, &header_);
    }

    Iterator end() {
        return Iterator(&header_, &header_);
    }

    Iterator end() const {
        return Iterator(&header_, &header_);
    }
// @}  // 红黑树的迭代器


// @{  // 在红黑树中查找元素的相关操作
private:

    // 在红黑树中查找键为`key`的节点
    NodeType *find_(NodeType *node, const K &key);

    /*
     * @brief 返回红黑树中序遍历序列中第一个 >= key 的节点。
     *
     * @param node 当前节点的指针。
     * @param key 要查找的键值。
     * @param satisfy_node 遍历到 node 节点时，上一次满足条件的节点
     *
     * @return 如果树中不存在键为`key`的节点，则返回第一个 > key 的节点。
     *         如果树中只有一个键为`key`的节点，则返回指向该节点的指针。
     *         如果树中存在多个键为`key`的节点，则返回指向中序遍历序列中第一个键为`key`的节点的指针。
     */
    NodeType *lower_bound_(NodeType *node, const K &key, NodeType *satisfy_node);

    /*
     * @brief 返回红黑树中序遍历序列中第一个 > key 的节点。
     *
     * @param node 当前节点的指针。
     * @param key 要查找的键值。
     * @param satisfy_node 遍历到 node 节点时，上一次满足条件的节点
     *
     * @return 如果树中不存在键为`key`的节点，则返回第一个 < key 的节点（注意注意注意）。
     *         如果树中只有一个键为`key`的节点，则返回指向该节点的指针。
     *         如果树中存在多个键为`key`的节点，则返回指向中序遍历序列中最后一个键为`key`的节点的指针。
     */
    NodeType *upper_bound_(NodeType *node, const K &key, NodeType *satisfy_node);

public:

    Iterator find(const K &key) {
        if (cnt_ == 0) return Iterator(&header_, &header_);

        auto root = static_cast<NodeType *>(header_.parent);
        auto node = find_(root, key);
        return Iterator(node == nullptr ? &header_ : node, &header_);
    }

    Iterator find(const K &key) const {
        return const_cast<RBTree*>(this)->find(key);
    }

    Iterator lower_bound(const K &key) {
        if (cnt_ == 0) return Iterator(&header_, &header_);

        auto root = static_cast<NodeType *>(header_.parent);
        auto node = lower_bound_(root, key, nullptr);
        return Iterator(node == nullptr ? &header_ : node, &header_);
    }

    Iterator lower_bound(const K &key) const {
        return const_cast<RBTree*>(this)->lower_bound(key);
    }

    Iterator upper_bound(const K &key) {
        if (cnt_ == 0) return Iterator(&header_, &header_);

        auto root = static_cast<NodeType *>(header_.parent);
        auto node = upper_bound_(root, key, nullptr);
        if (node == nullptr) {
            return this->begin();
        } else {
            Iterator ans = Iterator(node, &header_);
            if (node->key == key) return ans;
            else return ++ans;
        }
    }

    Iterator upper_bound(const K &key) const {
        return const_cast<RBTree*>(this)->upper_bound(key);
    }
// @}  // 在红黑树中查找元素的相关操作


public:
    SizeType size() const { return this->cnt_; }
};




// ----------------------RBTree类中部分函数的实现------------------------------
template<typename K, typename V, typename NodeType, typename Compare>
RBTree<K, V, NodeType, Compare>& RBTree<K, V, NodeType, Compare>::operator=(RBTree &&other) noexcept {
    if (this == &other) return *this;
    if (other.header_.parent == nullptr) return *this;

    auto p = other.header_.parent;
    p->parent = &header_;
    header_.parent = p;
    other.header_.parent = nullptr;

    header_.left = other.header_.left;
    header_.right = other.header_.right;
    other.header_.left = &other.header_;
    other.header_.right = &other.header_;

    cnt_ = other.cnt_;
    other.cnt_ = 0;
    return *this;
}

template<typename K, typename V, typename NodeType, typename Compare>
RBTree<K, V, NodeType, Compare>::RBTree(RBTree &&other) noexcept {
    if (other.header_.parent == nullptr) return;

    auto p = other.header_.parent;
    p->parent = &header_;
    header_.parent = p;
    other.header_.parent = nullptr;

    header_.left = other.header_.left;
    header_.right = other.header_.right;
    other.header_.left = &other.header_;
    other.header_.right = &other.header_;

    std::swap(cnt_, other.cnt_);
}

template<typename K, typename V, typename NodeType, typename Compare>
void RBTree<K, V, NodeType, Compare>::rotate_left_(NodeBase *node) {
    //     |                       |
    //     N                       S
    //    / \     l-rotate(N)     / \
    //   L   S    ==========>    N   R
    //      / \                 / \
    //     M   R               L   M
    if (node == nullptr || node->right == nullptr) {
        throw std::invalid_argument(
                "RBTree::rotate_left_: argument `node` or `node->right` "
                "can not be `nullptr`"
        );
    }

    NodeBase *parent = node->get_parent(&header_);
    auto r_son = node->right;

    // 修改 node 父节点的指向
    if (parent == nullptr) {
        header_.parent = r_son;
        r_son->parent = &header_;
    } else {
        if (parent->left == node) {
            parent->left = r_son;
        } else {
            parent->right = r_son;
        }
        r_son->parent = parent;
    }

    // 修改 node 右子树的指向
    node->right = r_son->left;
    if (r_son->left != nullptr)
        r_son->left->parent = node;

    // 修改 r_son 左子树的指向
    r_son->left = node;
    node->parent = r_son;
}

template<typename K, typename V, typename NodeType, typename Compare>
void RBTree<K, V, NodeType, Compare>::rotate_right_(NodeBase *node) {
    //       |                   |
    //       N                   S
    //      / \   r-rotate(N)   / \
    //     S   R  ==========>  L   N
    //    / \                     / \
    //   L   M                   M   R
    if (node == nullptr || node->left == nullptr) {
        throw std::invalid_argument(
                "RBTree::rotate_right_: argument `node` or `node->left`"
                " can not be `nullptr`"
        );
    }

    NodeBase *parent = node->get_parent(&header_);
    auto l_son = node->left;

    // 修改 node 父节点的指向
    if (parent == nullptr) {
        header_.parent = l_son;
        l_son->parent = &header_;
    } else {
        if (parent->left == node) {
            parent->left = l_son;
        } else {
            parent->right = l_son;
        }
        l_son->parent = parent;
    }

    // 修改 node 左子树的指向
    node->left = l_son->right;
    if (l_son->right != nullptr)
        l_son->right->parent = node;

    // 修改 l_son 右子树的指向
    l_son->right = node;
    node->parent = l_son;
}

template<typename K, typename V, typename NodeType, typename Compare>
void RBTree<K, V, NodeType, Compare>::rebalance_after_insert_(NodeType *node) {
    if (node == nullptr) return;

    // case1: 插入的节点为根节点 ==> 将该节点染`黑`
    if (header_.parent == node) {
        node->color = RBTreeColor::kBlack;
        return;
    }

    auto parent = static_cast<NodeType *>(node->get_parent(&header_));
    auto grandpa = static_cast<NodeType *>(parent->get_parent(&header_));
    auto uncle = static_cast<NodeType *>(node->get_uncle(&header_));

    // case2: 插入节点的父节点为黑 ==> 无需任何操作
    if (parent->color == RBTreeColor::kBlack) return;

    // case3: 父节点为红，且叔父节点为黑或为空 ==> 分LL, RR, LR, RL四种类型
    //        这种情况下，爷节点必然存在（不为空），因为`根叶黑`保证了根节点不为`红`
    if (uncle == nullptr || uncle->color == RBTreeColor::kBlack) {
        parent->color = parent->color == kRed ? kBlack : kRed;    // 父节点变色
        grandpa->color = grandpa->color == kRed ? kBlack : kRed;  // 爷节点变色
        if (grandpa->left == parent && parent->left == node) {
            // case3.1: LL型 ==> 父节点和爷节点变色，右旋爷节点
            rotate_right_(grandpa);  // 右旋爷节点
        } else if (grandpa->right == parent && parent->right == node) {
            // case3.2: RR型 ==> 父节点和爷节点变色，左旋爷节点
            rotate_left_(grandpa);  // 左旋爷节点
        } else if (grandpa->left == parent && parent->right == node) {
            // case3.3: LR型 ==> 父节点和爷节点变色，左旋父节点，右旋爷节点
            rotate_left_(parent);    // 左旋父节点
            rotate_right_(grandpa);  // 右旋爷节点
        } else {
            // case3.4: RL型 ==> 父节点和爷节点变色，右旋父节点，左旋爷节点
            rotate_right_(parent);  // 右旋父节点
            rotate_left_(grandpa);  // 左旋爷节点
        }
        return;
    }

    // case4: 父节点为红，叔父节点为红 ==> 叔、父、爷节点变色，爷节点变插入节点
    uncle->color = uncle->color == kRed ? kBlack : kRed;      // 叔节点变色
    parent->color = parent->color == kRed ? kBlack : kRed;    // 父节点变色
    grandpa->color = grandpa->color == kRed ? kBlack : kRed;  // 爷节点变色
    rebalance_after_insert_(grandpa);
}

template<typename K, typename V, typename NodeType, typename Compare>
template<typename ...Args>
NodeType *RBTree<K, V, NodeType, Compare>::insert_(NodeType *node, bool replace,
                                                   const K &key, Args &&... args) {
    if (node == nullptr) {
        throw std::invalid_argument(
                "RBTree::insert_: argument `node` can not be `nullptr`!");
    }

    if (node->key == key && replace) {
        node->replace_val(std::forward<Args>(args)...);
        return node;
    }

    if (cmp_(key, node->key)) {
        // key < node->key ==> 插入到 node 的左子树中
        if (node->left == nullptr) {
            auto tmp = NodeType::create_node(
                    key, std::forward<Args>(args)...);
            node->left = tmp;
            node->left->parent = node;
            rebalance_after_insert_(tmp);
            this->cnt_++;

            // 可能需要修改header_中left或right的指向
            if (header_.left == node)
                header_.left = node->left;
            return tmp;
        } else {
            return insert_(static_cast<NodeType *>(node->left),
                           replace, key, std::forward<Args>(args)...);
        }
    } else {
        // key >= node->key ==> 插入到 node 的右子树中
        if (node->right == nullptr) {
            auto tmp = NodeType::create_node(
                    key, std::forward<Args>(args)...);
            node->right = tmp;
            node->right->parent = node;
            rebalance_after_insert_(tmp);
            this->cnt_++;

            // 可能需要修改header_中left或right的指向
            if (header_.right == node)
                header_.right = node->right;
            return tmp;
        } else {
            return insert_(static_cast<NodeType *>(node->right),
                           replace, key, std::forward<Args>(args)...);
        }
    }
}

template<typename K, typename V, typename NodeType, typename Compare>
template<typename... Args>
typename RBTree<K, V, NodeType, Compare>::Iterator
RBTree<K, V, NodeType, Compare>::insert(bool replace, const K &key, Args &&... args) {
    NodeType *node;
    if (header_.parent == nullptr) {
        node = NodeType::create_node(
                key, std::forward<Args>(args)...
        );
        node->parent = &header_;
        node->color = RBTreeColor::kBlack;

        header_.parent = node;
        header_.left = header_.right = node;
        this->cnt_++;
    } else {
        auto root = static_cast<NodeType *>(header_.parent);
        node = this->insert_(root, replace, key, std::forward<Args>(args)...);
    }
    return Iterator(node, &header_);
}

template<typename K, typename V, typename NodeType, typename Compare>
void RBTree<K, V, NodeType, Compare>::destroy_node_(NodeType *node) {
    if (node == nullptr) {
        throw std::invalid_argument(
                "RBTree::destroy_node_: argument `node` can not be `nullptr`"
        );
    }

    NodeType::destroy_node(node, &header_);
    this->cnt_--;
}

template<typename K, typename V, typename NodeType, typename Compare>
void RBTree<K, V, NodeType, Compare>::clear_(NodeBase *node) {
    if (node == nullptr) return;

    this->clear_(node->left);   // 删除左子树
    this->clear_(node->right);  // 删除右子树
    this->destroy_node_(static_cast<NodeType *>(node));
}

template<typename K, typename V, typename NodeType, typename Compare>
bool RBTree<K, V, NodeType, Compare>::remove_(NodeType *node, const K &key) {
    if (node == nullptr) return false;

    if (key != node->key) {
        if (cmp_(key, node->key)) {
            // key < node->key
            auto l_son = static_cast<NodeType *>(node->left);
            return remove_(l_son, key);
        } else {
            // key > node->key
            auto r_son = static_cast<NodeType *>(node->right);
            return remove_(r_son, key);
        }
    }

    if (this->cnt_ == 1) {
        this->clear();
        return true;
    }

    // case 1: 节点 node 左右子树均不为空
    //     step 1: 寻找 node 的直接后继 successor
    //     step 2: 交换 node 和 successor 的内容（key 和 val）
    //     step 3: 转换为删除 successor
    //     |                    |
    //     N                    S
    //    / \                  / \
    //   L  ..   swap(N, S)   L  ..
    //       |   =========>       |
    //       P                    P
    //      / \                  / \
    //     S  ..                N  ..
    if (node->left != nullptr && node->right != nullptr) {
        // step 1: 寻找 node 的直接后继 successor
        NodeBase *tmp = node->right;
        while (tmp->left != nullptr) tmp = tmp->left;
        auto successor = static_cast<NodeType *>(tmp);

        // step 2: 交换 node 和 successor 的内容（key 和 val）
        successor->swap(node);

        // step 3: 转换为删除 successor
        node = successor;  // 直接跳转到后面两种情况即可
    }

    // case 2: node 有一个非空的左子树或右子树。由于红黑树的性质约束，
    // 此时 node 必为黑节点，且非空的左子树或右子树一定只有一个红节点
    //     step 1: 用非空的左子树或右子树代替掉 node 节点，子树变为黑
    //     step 2: 删除掉 node 节点
    if (node->left != nullptr || node->right != nullptr) {
        NodeBase *tmp = node->left == nullptr ? node->right : node->left;
        auto sub_tree = static_cast<NodeType *>(tmp);
        NodeBase *parent = node->get_parent(&header_);

        // step 1: 用非空的左子树或右子树代替掉 node 节点，子树变为黑
        if (parent == nullptr) {
            header_.parent = sub_tree;
            sub_tree->parent = &header_;
        } else {
            if (parent->left == node) {
                parent->left = sub_tree;
            } else {
                parent->right = sub_tree;
            }
            sub_tree->parent = parent;
        }
        sub_tree->color = RBTreeColor::kBlack;

        // step 2: 删除掉 node 节点
        if (header_.left == node) header_.left = sub_tree;
        if (header_.right == node) header_.right = sub_tree;
        this->destroy_node_(node);

        return true;
    }

    // case 3: node 节点为叶节点
    //     step 1: 删除 node 节点
    //     step 2: 如果 node 为黑节点，删除后需要调整红黑树
    if (node->color == RBTreeColor::kBlack) {
        // step 2: 如果 node 为黑节点，删除后需要调整红黑树
        // 删除后的调整操作 rebalance_after_remove_ 不会影响到节点本身
        // 因此，可以将 step 2 放到 step 1 之前执行
        rebalance_after_remove_(node);
    }

    // step 1: 删除 node 节点
    NodeBase *parent = node->parent;
    if (header_.left == node) header_.left = parent;
    if (header_.right == node) header_.right = parent;
    this->destroy_node_(node);

    return true;
}

// 删除节点 node 后，红黑树的调整策略
// 注意：首次调用rebalance_after_remove_时，node 为黑色叶节点
template<typename K, typename V, typename NodeType, typename Compare>
void RBTree<K, V, NodeType, Compare>::rebalance_after_remove_(NodeType *node) {
    if (header_.parent == node) return;

    auto sibling = static_cast<NodeType *>(node->get_sibling(&header_));

    // case 1: node 节点的兄弟节点为红色
    //     step 1: 如果 node 是左孩子，左旋父节点；否则，右旋父节点
    //     step 2: 父兄节点变色（父节点变红，兄节点变黑）
    //     step 3: 更新兄弟节点，并继续调整
    //      [P]                   <S>
    //      / \    l-rotate(P)    / \
    //    [N] <S>  ==========>  [P] [D]
    //        / \               / \
    //      [C] [D]           [N] [C]
    if (sibling->color == RBTreeColor::kRed) {
        auto parent = static_cast<NodeType *>(node->get_parent(&header_));
        // step 1: 如果 node 是左孩子，左旋父节点；否则，右旋父节点
        if (parent->left == node) rotate_left_(parent);
        else rotate_right_(parent);

        // step 2: 父兄节点变色（父节点变红，兄节点变黑）
        parent->color = RBTreeColor::kRed;
        sibling->color = RBTreeColor::kBlack;

        // step 3: 更新兄弟节点，并继续调整
        sibling = static_cast<NodeType *>(node->get_sibling(&header_));
    }

    auto parent = static_cast<NodeType *>(node->get_parent(&header_));
    NodeType *close_nephew, *distant_nephew;
    if (parent->left == node) {
        close_nephew = static_cast<NodeType *>(sibling->left);
        distant_nephew = static_cast<NodeType *>(sibling->right);
    } else {
        close_nephew = static_cast<NodeType *>(sibling->right);
        distant_nephew = static_cast<NodeType *>(sibling->left);
    }

    bool all_nephew_is_black = true;
    if (close_nephew != nullptr && close_nephew->color == kRed) {
        all_nephew_is_black = false;
    }
    if (distant_nephew != nullptr && distant_nephew->color == kRed) {
        all_nephew_is_black = false;
    }


    if (all_nephew_is_black) {
        if (parent->color == RBTreeColor::kRed) {
            // case 2: 兄弟节点为黑，侄子节点全为黑，且父节点为红
            //     将父节点和兄弟节点变色（父变黑，兄变红）
            //      <P>             [P]
            //      / \             / \
            //    [N] [S]  ====>  [N] <S>
            //        / \             / \
            //      [C] [D]         [C] [D]
            parent->color = RBTreeColor::kBlack;
            sibling->color = RBTreeColor::kRed;
            return;
        } else {
            // case 3: 兄弟节点、所有侄子节点和父节点全为黑
            //     step 1: 将兄弟节点变红
            //     step 2: 递归处理父节点
            //      [P]             [P]
            //      / \             / \
            //    [N] [S]  ====>  [N] <S>
            //        / \             / \
            //      [C] [D]         [C] [D]
            sibling->color = RBTreeColor::kRed;
            rebalance_after_remove_(parent);
            return;
        }
    } else {
        // case 4: 兄弟节点为黑色，离 node 最近的侄子节点是红色（LR型或RL型）
        //     step 1: 如果兄弟节点是左孩子，左旋兄弟节点；否则，右旋兄弟节点
        //     step 2: 交换最近的侄子节点和兄弟节点的颜色
        //     step 3: 转到 case 5
        //                            {P}
        //      {P}                   / \
        //      / \    r-rotate(S)  [N] <C>
        //    [N] [S]  ==========>        \
        //        / \                     [S]
        //      <C> [D]                     \
        //                                  [D]
        if (close_nephew != nullptr && close_nephew->color == kRed) {
            // step 1: 如果兄弟节点是左孩子，左旋兄弟节点；否则，右旋兄弟节点
            if (parent->left == sibling) {
                rotate_left_(sibling);
            } else rotate_right_(sibling);

            // step 2: 交换最近的侄子节点和兄弟节点的颜色
            sibling->color = RBTreeColor::kRed;
            close_nephew->color = RBTreeColor::kBlack;

            // 更新 sibling, close_nephew 和 distant_nephew
            sibling = static_cast<NodeType *>(node->get_sibling(&header_));
            if (parent->left == node) {
                close_nephew = static_cast<NodeType *>(sibling->left);
                distant_nephew = static_cast<NodeType *>(sibling->right);
            } else {
                close_nephew = static_cast<NodeType *>(sibling->right);
                distant_nephew = static_cast<NodeType *>(sibling->left);
            }
        }

        // case 5: 兄弟节点为黑色，最远的侄子节点是红色（LL型或RR型）
        //     step 1: 如果 node 是左孩子，左旋父节点；否则，右旋父节点
        //     step 2: 交换父节点和兄弟节点的颜色
        //     step 3: 将最远的侄子节点的颜色变为黑
        //      {P}                   [S]               {S}
        //      / \    l-rotate(P)    / \    repaint    / \
        //    [N] [S]  ==========>  {P} <D>  ======>  [P] [D]
        //        / \               / \               / \
        //      {C} <D>           [N] {C}           [N] {C}

        // step 1: 如果 node 是左孩子，左旋父节点；否则，右旋父节点
        if (parent->left == node) rotate_left_(parent);
        else rotate_right_(parent);

        // step 2: 交换父节点和兄弟节点的颜色
        sibling->color = parent->color;
        parent->color = RBTreeColor::kBlack;

        // step 3: 将最远的侄子节点的颜色变为黑
        if (distant_nephew != nullptr) {
            distant_nephew->color = RBTreeColor::kBlack;
        }
    }
}

template<typename K, typename V, typename NodeType, typename Compare>
NodeType *RBTree<K, V, NodeType, Compare>::find_(NodeType *node, const K &key) {
    if (node == nullptr) return nullptr;

    if (key == node->key) return node;
    else {
        auto l_son = static_cast<NodeType *>(node->left);
        auto r_son = static_cast<NodeType *>(node->right);
        if (cmp_(key, node->key)) {
            // key < node->key
            return find_(l_son, key);
        } else {
            // key > node->key
            return find_(r_son, key);
        }
    }
}

template<typename K, typename V, typename NodeType, typename Compare>
NodeType *RBTree<K, V, NodeType, Compare>::lower_bound_(
        NodeType *node, const K &key, NodeType *satisfy_node) {
    if (node == nullptr) return satisfy_node;

    auto l_son = static_cast<NodeType *>(node->left);
    auto r_son = static_cast<NodeType *>(node->right);
    if (!cmp_(node->key, key)) {
        // key <= node->key, 满足条件，继续走左孩子
        return lower_bound_(l_son, key, node);
    } else {
        // key > node->key, 不满足条件，走右孩子
        return lower_bound_(r_son, key, satisfy_node);
    }
}

template<typename K, typename V, typename NodeType, typename Compare>
NodeType *RBTree<K, V, NodeType, Compare>::upper_bound_(
        NodeType *node, const K &key, NodeType *satisfy_node) {
    if (node == nullptr) return satisfy_node;

    auto l_son = static_cast<NodeType *>(node->left);
    auto r_son = static_cast<NodeType *>(node->right);
    if (!cmp_(key, node->key)) {
        // key >= node->key, 满足条件，继续走右孩子
        return upper_bound_(r_son, key, node);
    } else {
        // key < node->key, 不满足条件，走左孩子
        return upper_bound_(l_son, key, satisfy_node);
    }
}


// 红黑树迭代器的实现 ********************************************************
template<typename K, typename V, typename NodeType, typename Compare>
struct RBTree<K, V, NodeType, Compare>::Iterator {
    NodeBase *node;
    const NodeBase *header;

    explicit Iterator(NodeBase *ptr, const NodeBase *h)
            : node(ptr), header(h) {}

    explicit Iterator(const NodeBase *ptr, const NodeBase *h)
            : node(const_cast<NodeBase *>(ptr)), header(h) {}

    ReturnType operator*() {
        return static_cast<NodeType *>(node)->get_data();
    }

    Iterator &operator++();

    Iterator operator++(int) noexcept {
        Iterator tmp = *this;
        ++*this;
        return tmp;
    }

    Iterator &operator--();

    Iterator operator--(int) {
        Iterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const Iterator &other) {
        return this->node == other.node;
    }

    bool operator!=(const Iterator &other) {
        return this->node != other.node;
    }
};

template<typename K, typename V, typename NodeType, typename Compare>
typename RBTree<K, V, NodeType, Compare>::Iterator &
RBTree<K, V, NodeType, Compare>::Iterator::operator++() {
    if (node->right != nullptr) {
        auto tmp = node->right;
        while (tmp->left != nullptr) tmp = tmp->left;
        node = tmp;
        return *this;
    }

    auto p = node->get_parent(header);
    if (p != nullptr && p->left == node) {
        node = p;
        return *this;
    }

    node = const_cast<NodeBase *>(header);
    return *this;
}

template<typename K, typename V, typename NodeType, typename Compare>
typename RBTree<K, V, NodeType, Compare>::Iterator &
RBTree<K, V, NodeType, Compare>::Iterator::operator--() {
    if (node->left != nullptr) {
        auto tmp = node->left;
        while (tmp->right != nullptr) tmp = tmp->right;
        node = tmp;
        return *this;
    }

    auto p = node->get_parent(header);
    if (p != nullptr && p->right == node) {
        node = p;
        return *this;
    }

    node = const_cast<NodeBase *>(header);
    return *this;
}

#endif //CPPBABYSTL_RB_TREE_H
