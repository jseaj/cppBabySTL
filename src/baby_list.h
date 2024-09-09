//
// Created by DELL on 2024/7/21.
//

#include <cstddef>
#include <utility>

#ifndef CPPBABYSTL_BABY_LIST_H
#define CPPBABYSTL_BABY_LIST_H

template<typename Tp>
class BList {
public:
    using SizeType = std::size_t;
    class Iterator;

private:
    // 定义两个辅助类
    struct NodeBase {
        NodeBase *prev, *next;

        NodeBase() : prev(this), next(this) {}
    };

    struct Node : public NodeBase {
        Tp data;

        Node() = default;
    };

    NodeBase head_;
    SizeType size_{};

private:



// @{  // 各类构造函数 / 析构函数
private:
    /*
     * @brief 销毁容器在范围[beg, end)内的元素
     * @param beg, end 要销毁的元素范围，NodeBase_指针类型
     *
     * 私有成员函数，供其它函数调用，减少代码冗余
     * */
    void destroy_range_(NodeBase *beg, NodeBase *end);

public:

    // 默认构造函数，创建一个空表
    BList() : size_(0) {}

    // 拷贝构造函数
    BList(const BList &other) : size_(0) {
        for (const auto &it: other) {
            this->emplace_back(it);
        }
    }

    // 移动构造函数
    BList(BList &&other) noexcept: size_(other.size_) {
        head_.next = other.head_.next;
        other.head_.next->prev = &head_;
        head_.prev = other.head_.prev;
        other.head_.prev->next = &head_;

        other.head_.next = &other.head_;
        other.head_.prev = &other.head_;
        other.size_ = 0;
    }

    // 创建长度为cnt的容器，值为类型Tp的默认值
    explicit BList(SizeType cnt) : size_(0) {
        for (SizeType i = 0; i < cnt; i++) {
            this->emplace_back();
        }
    }

    // 创建长度为cnt的容器，容器的值全为val
    BList(SizeType cnt, const Tp &val) : size_(0) {
        for (SizeType i = 0; i < cnt; i++) {
            this->emplace_back(val);
        }
    }

    // 通过初始化列表构造容器
    BList(std::initializer_list<Tp> init_list) : size_(0) {
        for (const auto &it: init_list) {
            this->emplace_back(it);
        }
    }

    // 析构函数
    ~BList() {
        this->destroy_range_(head_.next, &head_);
    }
// @}  // 各类构造函数 / 析构函数


// @{  // 与赋值相关操作
private:

    /*
     * @brief 将一个可迭代对象的范围[beg, end)的元素赋值给当前BList容器
     * @param beg 范围的起始迭代器
     * @param end 范围的结束迭代器（不包含）
     * */
    template<typename InputIter>
    void assign_range_(InputIter beg, InputIter end);

public:

    // 拷贝赋值函数
    BList & operator=(const BList &other) {
        if (this == &other) return *this;  // 防止自赋值
        assign_range_(other.begin(), other.end());
        return *this;
    }

    // 移动赋值函数
    BList & operator=(BList &&other) noexcept;

    // 将一个初始化列表中的元素赋值给当前容器
    BList & operator=(std::initializer_list<Tp> init_list) {
        assign_range_(init_list.begin(), init_list.end());
        return *this;
    }

    /*
     * @brief 用指定数量的元素值替换掉容器中的内容
     * @param cnt 替换后容器的大小
     * @param val 用以替换容器元素的值
     * */
    void assign(SizeType cnt, const Tp &val);

    // 将一个初始化列表的内容赋值当前容器
    void assign(std::initializer_list<Tp> init_list) {
        assign_range_(init_list.begin(), init_list.end());
    }
// @}  // 与赋值相关操作


// @{  // 与元素访问相关的操作
public:

    /*
     * @brief 返回容器首元素的引用
     *
     * 注意：在空容器上调用front()属于未定义的行为
     * */
    Tp & front() {
        auto node = static_cast<Node *>(head_.next);
        return node->data;
    }

    const Tp & front() const {
        auto node = static_cast<Node *>(head_.next);
        return node->data;
    }

    /*
     * @brief 返回容器中最后一个元素的引用
     *
     * 注意：在空容器上调用back()属于未定义的行为
     * */
    Tp & back() {
        auto node = static_cast<Node *>(head_.prev);
        return node->data;
    }

    const Tp & back() const {
        auto node = static_cast<Node *>(head_.prev);
        return node->data;
    }
// @}  // 与元素访问相关的操作


// @{  // 与容器容量相关的操作
public:

    // 返回容器的元素数
    SizeType size() const { return size_; }

    // 检查容器是否为空
    bool empty() const { return size_ == 0; }


// @}  // 与容器容量相关的操作


// @{  // 向容器中添加元素的相关操作
private:
    /*
     * @brief 创建一个节点
     * @param args 创建节点所需的参数
     * */
    template<typename ...Args>
    Node *create_node_(Args ...args) {
        auto node = new Node;
        new(&(node->data)) Tp(std::forward<Args>(args)...);
        return node;
    }

    /*
     * @brief 插入元素
     * @param pos 插入位置，新元素将插入到pos前面
     *
     * 私有成员函数，仅供其余函数调用
     * */
    template<typename ...Args>
    void insert_(NodeBase *pos, Args... args);

public:

    // 在容器尾部原位构造新元素
    template<typename ...Args>
    void emplace_back(Args... args) {
        insert_(&head_, std::forward<Args>(args)...);
    }

    // 在容器头部原位构造新元素
    template<typename ...Args>
    void emplace_front(Args... args) {
        insert_(head_.next, std::forward<Args>(args)...);
    }

    // 向容器末尾添加新元素
    void push_back(const Tp &val) {
        insert_(&head_, val);
    }

    void push_back(Tp &&val) {
        insert_(&head_, val);
    }

    // 在容器头部添加新元素
    void push_front(const Tp &val) {
        insert_(head_.next, val);
    }

    void push_front(Tp &&val) {
        insert_(head_.next, val);
    }

    /*
     * @brief 在容器的指定位置插入新元素
     * @param pos 元素的插入位置
     * @param val 要插入的元素值
     * */
    void insert(Iterator pos, const Tp &val) {
        insert_(pos.ptr, val);
    }

    /*
     * @brief 在容器的指定位置插入新元素
     * @param pos 元素的插入位置
     * @param val 要插入的元素值
     * */
    void insert(Iterator pos, Tp &&val) {
        insert_(pos.ptr, val);
    }

    /*
     * @brief 在容器的指定位置插入多个值为val的新元素
     * @param pos 元素的插入位置
     * @param cnt 要插入元素的数目
     * @param val 要插入的元素值
     * */
    void insert(Iterator pos, SizeType cnt, const Tp &val) {
        for (SizeType i = 0; i < cnt; i++) {
            insert_(pos.ptr, val);
        }
    }

    /*
     * @brief 在容器的指定位置插入一个初始化列表的所有元素
     * @param pos 元素的插入位置
     * @param init_list 要插入的值的来源初始化列表
     * */
    void insert(Iterator pos, std::initializer_list<Tp> init_list) {
        for (const auto &it: init_list) {
            insert_(pos.ptr, it);
        }
    }
// @}  // 向容器中添加元素的相关操作


// @{  // 在容器中删除元素的相关操作
public:

    // 清除容器中的所有元素
    void clear() {
        destroy_range_(head_.next, &head_);
    }

    // 删除指定位置的元素
    void erase(Iterator pos) {
        NodeBase *node = pos.ptr;
        destroy_range_(node, node->next);
    }

    // 删除[beg, end)内的所有元素
    void erase(Iterator beg, Iterator end) {
        NodeBase *node_beg = beg.ptr;
        NodeBase *node_end = end.ptr;
        destroy_range_(node_beg, node_end);
    }

    /*
     * @brief 移除首元素
     *
     * 与std::list不同，当容器为空时，该操作不会有任何行为
     * 而std::list中，当容器为空时，pop_front属于为定义的行为
     * */
    void pop_front() {
        destroy_range_(head_.next, head_.next->next);
    }

    /*
     * @brief 移除末尾元素
     *
     * 与std::list不同，当容器为空时，该操作不会有任何行为
     * 而std::list中，当容器为空时，pop_back属于为定义的行为
     * */
    void pop_back() {
        destroy_range_(head_.prev, &head_);
    }
// @}  // 在容器中删除元素的相关操作


// @{  // 与内容修改相关的操作
public:
    /*
     * @brief 重设容器以容纳cnt个元素大小
     * @param cnt 容器的大小
     * */
    void resize(SizeType cnt) {
        NodeBase *cur = head_.next;
        for (SizeType i = 0; i < cnt; i++) {
            if (cur == &head_) {
                insert_(cur);
            } else {
                cur = cur->next;
            }
        }
        // 销毁掉多余空间
        destroy_range_(cur, &head_);
    }

    /*
     * @brief 重设容器以容纳cnt个元素大小
     * @param cnt 容器的大小
     * @param val 用以初始化新元素的值
     * */
    void resize(SizeType cnt, const Tp &val) {
        NodeBase *cur = head_.next;
        for (SizeType i = 0; i < cnt; i++) {
            if (cur == &head_) {
                insert_(cur, val);
            } else {
                cur = cur->next;
            }
        }
        // 销毁掉多余空间
        destroy_range_(cur, &head_);
    }

    // 将当前容器的内容与other交换
    void swap(BList &other) noexcept;
// @}  // 与内容修改相关的操作


// @{  // 链表独有的相关操作
public:

    /*
     * @brief 合并两个有序的BList容器
     * @param other 待合并的另一个有序的BList容器
     * @param cmp 可调用对象，用于比较两个Tp类型对象的关系
     *
     * 操作完成后，other变为空；如果待合并的两个BList
     * 不是有序的，则属于未定义的行为
     * */
    template<class Compare>
    void merge(BList &other, Compare cmp);

    template<class Compare>
    void merge(BList &&other, Compare cmp) {
        this->merge(other, cmp);
    }

    void merge(BList &other) {
        this->merge(other, std::less<Tp>());
    }

    void merge(BList &&other) {
        this->merge(other, std::less<Tp>());
    }

    // 移除容器中所有值为val的元素
    void remove(const Tp &val);

    /*
     * @brief 移除容器中所有满足某个条件的值
     * @param p 可调用对象，若元素elem满足条件则p(elem)==true，否则为false
     * */
    template<class Predicate>
    void remove_if(Predicate p);

    /*
     * @brief 反转容器中的元素顺序
     *
     * 与单链表的操作有所不同，在双链表中只需要将每个
     * 节点的next指针和prev指针指向的位置相互交换即可
     * */
    void reverse() noexcept {
        NodeBase *cur = head_.next;
        while (cur != &head_) {
            NodeBase *tmp = cur;
            cur = cur->next;
            std::swap(tmp->prev, tmp->next);
        }
        std::swap(head_.prev, head_.next);
    }

    /*
     * @brief 移除容器中所有相继相等（自定义的相等）的元素
     * @param p 可调用对象，用于判断容器内的两个元素是否相等
     * */
    template<class Predicate>
    void unique(Predicate p);

    // 移除容器中所有相继相等的元素
    void unique() {
        this->unique(std::equal_to<Tp>());
    }

    /*
     * @brief 对容器内的元素进行排序
     * @param cmp 可调用对象，比较函数
     *
     * 非递归版本的归并排序，时间复杂度O(nlogn)，空间复杂度O(1)
     * */
    template<class Compare>
    void sort(Compare cmp);

    void sort() {
        this->sort(std::less<Tp>());
    }
// @}  // 链表独有的相关操作


// @{  // 与迭代器相关的操作
public:
    Iterator begin() {
        return Iterator(head_.next);
    }

    Iterator begin() const {
        return Iterator(head_.next);
    }

    Iterator end() {
        return Iterator(&head_);
    }

    Iterator end() const {
        return Iterator(&head_);
    }
// @}  // 与迭代器相关的操作
};


// @{  // 与BList相关的非成员函数
// 重载==运算符
template<typename T>
bool
operator==(const BList<T> &x, const BList<T> &y) {
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
// @}  // 与BList相关的非成员函数


// @{  // BList类迭代器的实现
template<typename Tp>
class BList<Tp>::Iterator {
private:
    NodeBase *ptr;
    friend BList;
public:
    explicit Iterator(NodeBase *p) : ptr(p) {}

    explicit Iterator(const NodeBase *p) {
        ptr = const_cast<NodeBase *>(p);
    }

    const Tp & operator*() const {
        return static_cast<Node *>(ptr)->data;
    }

    Tp & operator*() {
        return static_cast<Node *>(ptr)->data;
    }

    const Iterator & operator++() const {
        ptr = ptr->next;
        return *this;
    }

    Iterator & operator++() {
        ptr = ptr->next;
        return *this;
    }

    Iterator operator++(int) {
        Iterator tmp = *this;
        ptr = ptr->next;
        return tmp;
    }

    const Iterator & operator--() const {
        ptr = ptr->prev;
        return *this;
    }

    Iterator & operator--() {
        ptr = ptr->prev;
        return *this;
    }

    Iterator operator--(int) {
        Iterator tmp = *this;
        ptr = ptr->prev;
        return tmp;
    }

    bool operator==(const Iterator &other) const {
        return ptr == other.ptr;
    }

    bool operator!=(const Iterator &other) const {
        return ptr != other.ptr;
    }
};
// @}  // BList类迭代器的实现


// @{  // BList中声明但是没有实现的成员函数
template<typename Tp>
template<typename... Args>
void BList<Tp>::insert_(BList::NodeBase *pos, Args... args) {
    Node *tmp = create_node_(std::forward<Args>(args)...);

    // 将tmp插入到pos前
    tmp->prev = pos->prev;
    pos->prev->next = tmp;
    tmp->next = pos;
    pos->prev = tmp;

    this->size_++;
}

template<typename Tp>
void BList<Tp>::destroy_range_(BList::NodeBase *beg, BList::NodeBase *end) {
    while (beg != end) {
        auto tmp = static_cast<Node *>(beg);
        beg = beg->next;

        // 断开连接
        tmp->prev->next = beg;
        beg->prev = tmp->prev;

        // 容器大小减1
        size_--;

        // 销毁tmp指向的内存
        delete tmp;
    }
}

template<typename Tp>
template<typename InputIter>
void BList<Tp>::assign_range_(InputIter beg, InputIter end) {
    NodeBase *cur = head_.next;
    while (beg != end) {
        if (cur == &head_) {
            // 当前容器的容量不足
            insert_(cur, *beg);
        } else {
            auto tmp = static_cast<Node *>(cur);
            cur = cur->next;
            (tmp->data).~Tp();  // 析构掉当前对象
            tmp->data = *beg;   // 赋值为新元素
        }
        ++beg;
    }

    // 销毁掉多余元素
    destroy_range_(cur, &head_);
}

template<typename Tp>
BList<Tp> & BList<Tp>::operator=(BList &&other) noexcept {
    // 销毁当前容器
    destroy_range_(head_.next, &head_);

    // 执行移动赋值操作
    head_.next = other.head_.next;
    other.head_.next->prev = &head_;
    head_.prev = other.head_.prev;
    other.head_.prev->next = &head_;

    other.head_.next = &other.head_;
    other.head_.prev = &other.head_;
    size_ = other.size_;
    other.size_ = 0;
}

template<typename Tp>
void BList<Tp>::assign(BList::SizeType cnt, const Tp &val) {
    NodeBase *cur = head_.next;
    for (SizeType i = 0; i < cnt; i++) {
        if (cur == &head_) {
            // 当前容器容量不足
            insert_(cur, val);
        } else {
            auto tmp = static_cast<Node *>(cur);
            cur = cur->next;
            (tmp->data).~Tp();  // 析构掉当前对象
            tmp->data = val;    // 新元素赋值给当前对象
        }
    }
    destroy_range_(cur, &head_);  // 删除掉多余元素
}

template<typename Tp>
void BList<Tp>::swap(BList &other) noexcept {
    NodeBase *head = head_.next;
    NodeBase *tail = head_.prev;

    head_.next = other.head_.next;
    other.head_.next->prev = &head_;
    head_.prev = other.head_.prev;
    other.head_.prev->next = &head_;

    other.head_.next = head;
    head->prev = &other.head_;
    other.head_.prev = tail;
    tail->next = &other.head_;

    std::swap(size_, other.size_);
}

template<typename Tp>
template<class Compare>
void BList<Tp>::merge(BList &other, Compare cmp) {
    if (this == &other) return;

    NodeBase *l1 = head_.next;
    NodeBase *l2 = other.head_.next;

    while ((l1 != &head_) && (l2 != &other.head_)) {
        if (cmp(static_cast<Node *>(l1)->data, static_cast<Node *>(l2)->data)) {
            l1 = l1->next;
        } else {
            auto tmp = static_cast<Node *>(l2);
            l2 = l2->next;

            // 节点tmp断开连接
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;

            // 将tmp插入到l1的前面
            tmp->prev = l1->prev;
            l1->prev->next = tmp;
            tmp->next = l1;
            l1->prev = tmp;
        }
    }
    if (l2 != &other.head_) {
        l2->prev = l1->prev;
        l1->prev->next = l2;
        other.head_.prev->next = l1;
        l1->prev = other.head_.prev;

        // 修改容器other头节点的指向
        other.head_.next = &other.head_;
        other.head_.prev = &other.head_;
    }
    size_ += other.size_;
    other.size_ = 0;
}

template<typename Tp>
void BList<Tp>::remove(const Tp &val) {
    NodeBase *cur = head_.next;
    NodeBase *extra = nullptr;
    while (cur != &head_) {
        NodeBase *tmp = nullptr;
        if (static_cast<Node *>(cur)->data == val) {
            if (&(static_cast<Node *>(cur)->data) == &val) {
                extra = cur;
            } else {
                tmp = cur;
            }
        }
        cur = cur->next;
        if (tmp != nullptr) {
            destroy_range_(tmp, tmp->next);
        }
    }
    if (extra != nullptr) {
        destroy_range_(extra, extra->next);
    }
}

template<typename Tp>
template<class Predicate>
void BList<Tp>::remove_if(Predicate p) {
    NodeBase *cur = head_.next;
    while (cur != &head_) {
        NodeBase *tmp = nullptr;
        if (p(static_cast<Node *>(cur)->data)) {
            tmp = cur;
        }
        cur = cur->next;
        if (tmp != nullptr) {
            destroy_range_(tmp, tmp->next);
        }
    }
}

template<typename Tp>
template<class Predicate>
void BList<Tp>::unique(Predicate p) {
    NodeBase *cur = head_.next->next;
    while (cur != &head_) {
        NodeBase *tmp = nullptr;
        if (cur->prev == &head_) {
            cur = cur->next;
            continue;
        }
        if (p(static_cast<Node *>(cur->prev)->data, static_cast<Node *>(cur)->data)) {
            tmp = cur;
        }
        cur = cur->next;
        if (tmp != nullptr) {
            destroy_range_(tmp, tmp->next);
        }
    }
}

template<typename Tp>
template<class Compare>
void BList<Tp>::sort(Compare cmp) {
    SizeType merge_len = 1;
    while (merge_len < size_) {
        NodeBase *p = head_.next;
        while (p != &head_) {
            // 寻找两个归并段的头和尾
            NodeBase *h1 = p, *h2;
            NodeBase *t1, *t2;
            for (SizeType i = 0; i < merge_len && p != &head_; i++) {
                p = p->next;
            }
            if (p == &head_) continue;
            h2 = t1 = p;
            for (SizeType i = 0; i < merge_len && p != &head_; i++) {
                p = p->next;
            }
            t2 = p;

            // 归并两个有序段[h1, t1)和[h2, t2)
            while (h1 != t1 && h2 != t2) {
                if (cmp(static_cast<Node *>(h1)->data, static_cast<Node *>(h2)->data)) {
                    h1 = h1->next;
                } else {
                    NodeBase *tmp = h2;
                    h2 = h2->next;

                    // 断开tmp的原有连接
                    tmp->prev->next = h2;
                    h2->prev = tmp->prev;

                    // 将tmp插入到h1前
                    tmp->prev = h1->prev;
                    h1->prev->next = tmp;
                    tmp->next = h1;
                    h1->prev = tmp;
                }
            }
        }
        merge_len *= 2;
    }
}
// @}  // BList中声明但是没有实现的成员函数

#endif //CPPBABYSTL_BABY_LIST_H
