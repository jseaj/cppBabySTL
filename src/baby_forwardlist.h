//
// Created by DELL on 2024/7/16.
//

#include <cstddef>

#ifndef CPPBABYSTL_BABY_FORWARDLIST_H
#define CPPBABYSTL_BABY_FORWARDLIST_H

template<typename Tp>
class BForwardList {
public:
    using SizeType = std::size_t;

private:
    // 定义两个辅助结构
    struct NodeBase {
        NodeBase* next;
        NodeBase(): next(nullptr) {}
    };

    struct Node: public NodeBase {
        Tp storage;
        Node(): NodeBase() {}
    };

    NodeBase head_;
    SizeType len_;


// @{  // 各类构造函数 / 析构函数
private:

    // 创建一个存储元素的节点
    template<typename... Args>
    static Node* create_node_(Args&&... args) {
        auto node = new Node;

        // placement new创建对象
        new (&(node->storage)) Tp(std::forward<Args>(args)...);
        return node;
    }

    /*
     * @brief 销毁容器在(beg, end)之间的所有元素
     * @param beg 销毁操作的起始点，即要销毁的第一个元素的前一个元素的指针
     * @param end 销毁操作的结束点，即要销毁的最后一个元素的后一个元素的指针
     * */
    void destroy_range_(NodeBase* beg, NodeBase* end) {
        NodeBase *cur = beg->next;
        while (cur != end) {
            auto tmp = static_cast<Node*>(cur);
            cur = cur->next;
            delete tmp;
            len_--;  // 删除一个节点，长度减少1
        }
        beg->next = end;
    }
public:

    // 默认构造函数
    BForwardList(): len_(0) {}

    // 拷贝构造函数
    BForwardList(const BForwardList& other): len_(other.size()) {
        NodeBase* p = &head_;
        for (const auto& it : other) {
            p->next = this->create_node_(it);
            p = p->next;
        }
    }

    // 移动构造函数
    BForwardList(BForwardList&& other) noexcept {
        len_ = other.size();
        head_.next = other.head_.next;

        // 修改 other 的值
        other.len_ = 0;
        other.head_.next = nullptr;
    }

    // 构造一个长度为 cnt，值全为默认值的容器
    explicit
    BForwardList(SizeType cnt): len_(cnt) {
        NodeBase *p = &head_;
        for (SizeType i = 0; i < cnt; i++) {
            p->next = this->create_node_();
            p = p->next;
        }
    }

    // 构造一个长度为 cnt，值全为 val的容器
    BForwardList(SizeType cnt, const Tp& val): len_(cnt) {
        NodeBase *p = &head_;
        for (SizeType i = 0; i < cnt; i++) {
            p->next = this->create_node_(val);
            p = p->next;
        }
    }

    // 通过初始化列表构造容器
    BForwardList(std::initializer_list<Tp> init_list)
            : len_(init_list.size()) {
        NodeBase *p = &head_;
        for (const auto &it: init_list) {
            p->next = this->create_node_(it);
            p = p->next;
        }
    }

    // 析构函数
    ~BForwardList() noexcept {
        destroy_range_(&head_, nullptr);
    }
// @}  // 各类构造函数 / 析构函数


// @{  // 与赋值相关的操作
private:
    /*
     * @brief 用迭代器[beg, end)中的内容完全替换掉容器中的内容
     * @param beg 指向第一个要替换元素的迭代器
     * @param end 指向最后一个要替换元素的下一个位置的迭代器
     *
     * 供重载=运算符和assign函数调用，用以减少冗余代码
     * */
    template<typename InputIter>
    void assign_(InputIter beg, InputIter end);
public:

    // 拷贝赋值函数
    BForwardList& operator=(const BForwardList& other) {
        if (this == &other) return *this;  // 防止自赋值

        assign_(other.begin(), other.end());
        return *this;
    }

    // 移动赋值函数
    BForwardList& operator=(BForwardList&& other) noexcept {
        len_ = other.len_;
        head_.next = other.head_.next;

        other.len_ = 0;
        other.head_.next = nullptr;
    }

    // 将一个初始化列表中的内容赋值给容器
    BForwardList& operator=(std::initializer_list<Tp> init_list) {
        assign_(init_list.begin(), init_list.end());
        return *this;
    }

    // 将容器中的内容替换为长度为 cnt，值为 val
    void assign(SizeType cnt, const Tp& val);

    /*
     * @brief 以范围[beg, end)中元素的副本替换内容
     * @param beg, end 复制来源元素的范围
     *
     * [beg, end)中有任何一个实参是指向*this中的迭代器时行为未定义
     * */
    template<typename InputIter>
    void assign(InputIter beg, InputIter end) {
        assign_(beg, end);
    }

    // 以来自初始化列表中的内容替换掉容器中的内容
    void assign(std::initializer_list<Tp> init_list) {
        assign_(init_list.begin(), init_list.end());
    }
// @}  //  与赋值相关的操作


// @{  // 与容量相关的成员函数
public:

    // 返回容器的容量
    SizeType size() const { return this->len_; }

    // 判断容器是否为空
    bool empty() { return this->len_ == 0; }
// @}  // 与容量相关的成员函数


// @{  // 向容器中添加元素的操作
public:

    // 在容器头原位构造元素
    template<typename... Args>
    void emplace_front(Args&&... args) {
        auto node = create_node_(std::forward<Args>(args)...);
        node->next = head_.next;
        head_.next = node;

        len_++;
    }

    // 添加元素到容器头
    void push_front(const Tp& val) {
        this->emplace_front(val);
    }

    // 添加元素到容器头
    void push_front(Tp&& val) {
        this->push_front(val);
    }
// @}  // 向容器中添加元素的操作


// @{  // 在容器中删除元素的相关操作
public:

    // 移除容器的首元素
    void pop_front() {
        if (this->empty()) return;

        auto node = static_cast<Node*>(head_.next);
        head_.next = node->next;
        delete node;
        len_--;
    }

    /*
     * @brief 清楚容器的所有元素
     *
     * 该函数只是清除元素，如果元素本身是指针，不会销毁掉
     * 指针所指的内存空间
     * */
    void clear() noexcept {
        destroy_range_(&head_, nullptr);
    }

    /*
     * @brief 重设容器的大小以容纳 cnt 个元素
     * @param cnt 容器的大小
     *
     * 如果容器当前大小为 cnt，不做任何操作
     * 如果容器当前大小小于 cnt，追加额外的默认插入元素
     * 如果容器当前大小大于 cnt，减少容器到它前 cnt 个元素
     * */
    void resize(SizeType cnt) {
        NodeBase* pre = &head_;
        for (SizeType i = 0; i < cnt; i++) {
            if (pre->next == nullptr) {
                // 追加默认元素
                pre->next = create_node_();
            }
            pre = pre->next;
        }
        // 清除掉多余元素
        destroy_range_(pre, nullptr);
        len_ = cnt;
    }

    /*
     * @brief 重设容器的大小以容纳 cnt 个元素
     * @param cnt 容器的大小
     * @param val 用以初始化新元素的值
     *
     * 如果容器当前大小为 cnt，不做任何操作
     * 如果容器当前大小小于 cnt，追加额外的值为 val 的元素
     * 如果容器当前大小大于 cnt，减少容器到它前 cnt 个元素
     * */
    void resize(SizeType cnt, const Tp& val) {
        NodeBase* pre = &head_;
        for (SizeType i = 0; i < cnt; i++) {
            if (pre->next == nullptr) {
                // 追加值为 val 的新元素
                pre->next = create_node_(val);
            }
            pre = pre->next;
        }
        // 清除掉多余元素
        destroy_range_(pre, nullptr);
        len_ = cnt;
    }

    /*
     * @brief 将当前容器的内容与另一容器 other 交换
     * @param other 要与之交换内容的容器
     *
     * 不在单独的元素上调用任何移动、复制或交换操作：时间复杂度O(1)
     * */
    void swap(BForwardList& other) noexcept {
        std::swap(head_.next, other.head_.next);
        std::swap(len_, other.len_);
    }
// @}  // 在容器中删除元素的相关操作


// @{  // 链表容器的独有操作
public:

    /*
     * @brief 合并两个有序链表
     * @param other 待合并的另一容器
     * @param cmp 比较函数（可调用对象）
     *
     * 如果other与*this是同一对象，那么什么也不做；合并操作完成后
     * other会变空
     * */
    template<class Compare>
    void merge(BForwardList& other, Compare cmp);

    template<class Compare>
    void merge(BForwardList&& other, Compare cmp) {
        this->merge(other, cmp);
    }

    void merge(BForwardList& other) {
        this->merge(other, std::less<Tp>());
    }

    void merge(BForwardList&& other) {
        this->merge(other, std::less<Tp>());
    }

    /*
     * @brief 从容器中删除所有值等于 val 的元素
     * @param val 要移除元素的值
     *
     * 注意：val可能是容器中某个元素的引用，这种情况需要
     * 特殊考虑，即最后再删除该元素
     * */
    void remove(const Tp& val);

    /*
     * @brief 删除所有满足条件的元素
     * @param p 可调用对象，若应该移除该元素则返回true的一元谓词
     * */
    template<class Predicate>
    void remove_if(Predicate p);

    // 反转容器中的元素顺序
    void reverse() noexcept;

    /*
     * @brief 从容器中移除所有相继地重复元素
     * @param p 判断两个元素是否相等的可调用对象
     * */
    template<class Predicate>
    void unique(Predicate p);

    // 从容器中移除所有相继地重复元素
    void unique() {
        this->unique(std::equal_to<Tp>());
    }

    /*
     * @brief 排序元素，并保持等价元素的顺序（稳定排序）
     * @param cmp 可调用对象，比较函数
     *
     * 链表的归并排序：时间复杂度O(nlogn)，空间复杂度O(1)
     * */
    template<class Compare>
    void sort(Compare cmp);

    void sort() {
        this->sort(std::less<Tp>());
    }
// @}  // 链表容器的独有操作


// @{  // 迭代器相关的操作
public:
    class Iterator;  // 迭代器类的声明

    Iterator begin() {
        return Iterator(head_.next);
    }

    Iterator begin() const {
        return Iterator(head_.next);
    }

    Iterator end() {
        return Iterator(static_cast<NodeBase*>(nullptr));
    }

    Iterator end() const {
        return Iterator(static_cast<NodeBase*>(nullptr));
    }
// @}  // 与迭代器相关的操作
};


// @{  // 迭代器类的实现
template<typename Tp>
class BForwardList<Tp>::Iterator {
private:
    NodeBase *cur;

public:
    explicit Iterator(NodeBase *node) : cur(node) {}

    explicit Iterator(const NodeBase *node)
            : cur(const_cast<NodeBase *>(node)) {}

    // 解引用
    Tp & operator*() {
        return static_cast<Node*>(cur)->storage;
    }

    const Tp & operator*() const {
        return static_cast<Node*>(cur)->storage;
    }

    // 重载前置++操作符
    Iterator & operator++() {
        cur = cur->next;
        return *this;
    }

    // 重载后置++操作符
    Iterator operator++(int) {
        Iterator tmp = *this;
        ++*this;
        return tmp;
    }

    // 重载==操作符
    bool operator==(const Iterator& other) const {
        return this->cur == other.cur;
    }

    // 重载!=操作符
    bool operator!=(const Iterator& other) const {
        return this->cur != other.cur;
    }
};
// @}  // 迭代器类的实现


// @{  // BForwardList类的非成员函数
// 比较两个容器是否相等
template<typename Tp>
bool operator==(const BForwardList<Tp>& x, const BForwardList<Tp>& y) {
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
// @}  // BForwardList类的非成员函数


// @{  // 类 BForwardList 内声明但未实现的成员函数
template<typename Tp>
template<typename InputIter>
void BForwardList<Tp>::assign_(InputIter beg, InputIter end) {
    len_ = 0;
    NodeBase* pre = &head_;
    while (beg != end) {
        if (pre->next == nullptr) {
            pre->next = create_node_(*beg);
        } else {
            auto tmp = static_cast<Node*>(pre->next);
            (tmp->storage).~Tp();  // 调用对象的析构函数
            tmp->storage = *beg;
        }
        ++beg;
        pre = pre->next;
        len_++;
    }

    // 删除掉多余的元素
    destroy_range_(pre, nullptr);
}

template<typename Tp>
void BForwardList<Tp>::assign(BForwardList::SizeType cnt, const Tp &val) {
    NodeBase* pre = &head_;
    for (SizeType i = 0; i < cnt; i++) {
        if (pre->next == nullptr) {
            pre->next = this->create_node_(val);
        } else {
            auto tmp = static_cast<Node*>(pre->next);
            (&(tmp->storage))->~Tp();  // 调用析构函数
            tmp->storage = val;
        }
        pre = pre->next;
    }

    // 销毁掉多余的元素
    destroy_range_(pre, nullptr);
    len_ = cnt;
}

template<typename Tp>
template<class Compare>
void BForwardList<Tp>::merge(BForwardList &other, Compare cmp) {
    if (this == &other) return;

    auto l1 = static_cast<Node*>(head_.next);
    auto l2 = static_cast<Node*>(other.head_.next);
    head_.next = nullptr;
    other.head_.next = nullptr;

    NodeBase* pre = &head_;
    while (l1 != nullptr && l2 != nullptr) {
        // l1 和 l2 中通过 cmp 比较获胜的节点
        Node* tmp = nullptr;
        if (cmp(l1->storage, l2->storage)) {
            tmp = l1;
            l1 = static_cast<Node*>(l1->next);
        } else {
            tmp = l2;
            l2 = static_cast<Node*>(l2->next);
        }

        // 将tmp插入到当前位置
        pre->next = tmp;
        pre = pre->next;
    }

    if (l1 != nullptr) {
        pre->next = l1;
    }
    if (l2 != nullptr) {
        pre->next = l2;
    }

    // 修改长度
    len_ += other.len_;
    other.len_ = 0;
}

template<typename Tp>
void BForwardList<Tp>::remove(const Tp &val) {
    NodeBase* pre = &head_;
    NodeBase* extra = nullptr;  // 需要特殊考虑的情况

    while (pre->next != nullptr) {
        auto tmp = static_cast<Node*>(pre->next);
        if (tmp->storage == val) {
            if (&tmp->storage != &val) {
                // 直接删除即可
                destroy_range_(pre, tmp->next);
                continue;
            } else {
                extra = pre;
            }
        }
        pre = pre->next;
    }

    if (extra != nullptr) {
        destroy_range_(extra, extra->next->next);
    }
}

template<typename Tp>
template<class Predicate>
void BForwardList<Tp>::remove_if(Predicate p) {
    NodeBase* pre = &head_;
    while (pre->next != nullptr) {
        auto cur = static_cast<Node*>(pre->next);
        if (p(cur->storage)) {
            destroy_range_(pre, cur->next);
        } else {
            pre = pre->next;
        }
    }
}

template<typename Tp>
void BForwardList<Tp>::reverse() noexcept {
    NodeBase *cur = head_.next;
    head_.next = nullptr;

    // 头插法反转顺序
    while (cur != nullptr) {
        Node* tmp = static_cast<Node*>(cur);
        cur = cur->next;

        tmp->next = head_.next;
        head_.next = tmp;
    }
}

template<typename Tp>
template<class Predicate>
void BForwardList<Tp>::unique(Predicate p) {
    NodeBase* pre = &head_;
    Node* preVal = nullptr;

    while (pre->next != nullptr) {
        auto cur = static_cast<Node*>(pre->next);
        if (preVal != nullptr && p(preVal->storage, cur->storage)) {
            // 删除当前的元素
            destroy_range_(pre, cur->next);
            continue;
        } else {
            preVal = cur;
        }
        pre = pre->next;
    }
}

template<typename Tp>
template<class Compare>
void BForwardList<Tp>::sort(Compare cmp) {
    SizeType len = this->size();
    SizeType merge_size = 1;  // 每一趟归并排序的长度

    while (merge_size < len) {
        auto p = static_cast<Node*>(head_.next);
        NodeBase* pre = &head_;

        while (p != nullptr) {
            NodeBase h1, h2;  // 带归并的两个头节点
            NodeBase *ph1 = &h1, *ph2 = &h2;
            for (SizeType i = 0; i < merge_size && p != nullptr; i++) {
                ph1->next = p;
                ph1 = ph1->next;
                p = static_cast<Node*>(p->next);
            }
            for (SizeType i = 0; i < merge_size && p != nullptr; i++) {
                ph2->next = p;
                ph2 = ph2->next;
                p = static_cast<Node*>(p->next);
            }
            ph1->next = nullptr;
            ph2->next = nullptr;

            // 归并两个有序链表
            auto p1 = static_cast<Node*>(h1.next);
            auto p2 = static_cast<Node*>(h2.next);
            while (p1 != nullptr && p2 != nullptr) {
                Node* tmp;
                if (cmp(p1->storage, p2->storage)) {
                    tmp = p1;
                    p1 = static_cast<Node*>(p1->next);
                } else {
                    tmp = p2;
                    p2 = static_cast<Node*>(p2->next);
                }

                // 将tmp插入到pre后
                pre->next = tmp;
                pre = pre->next;
            }
            if (p1 != nullptr) {
                pre->next = p1;
                pre = ph1;
            }
            if (p2 != nullptr) {
                pre->next = p2;
                pre = ph2;
            }
        }
        merge_size *= 2;
    }
}
// @}  // 类 BForwardList 内声明但未实现的成员函数

#endif //CPPBABYSTL_BABY_FORWARDLIST_H
