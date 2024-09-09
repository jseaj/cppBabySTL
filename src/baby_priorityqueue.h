//
// Created by DELL on 2024/8/7.
//

#include "baby_vector.h"

#ifndef CPPBABYSTL_BABY_PRIORITYQUEUE_H
#define CPPBABYSTL_BABY_PRIORITYQUEUE_H


template<typename Tp, typename Container=BVector<Tp>,
         typename Compare=std::less<Tp>>
class BPriorityQueue {
public:
    using SizeType = typename Container::SizeType;

private:
    Container container_;  // 优先队列中存储数据的容器
    Compare cmp_;          // 比较函数


// @{  // 各类构造函数
private:
    /*
     * @brief 将容器container_内的元素堆化
     *
     * 从叶子节点开始，通过向下调整（adjust_down_）挨个将
     * 元素堆化，时间复杂度为O(n)，此处n表示容器的大小，
     * 证明详见：https://oi-wiki.org/ds/binary-heap/
     * */
    void heapify_() {
        SizeType n = container_.size();
        for (SizeType i = n; i > 0; --i) {
            adjust_down_(i - 1);
        }
    }

public:

    // 默认构造函数
    BPriorityQueue()
            : container_(), cmp_() {}

    // 拷贝构造函数
    BPriorityQueue(const BPriorityQueue &other)
            : container_(other.container_),
              cmp_(other.cmp_) {}

    // 移动构造函数
    BPriorityQueue(BPriorityQueue &&other) noexcept
            : container_(std::move(other.container_)),
              cmp_(std::move(other.cmp_)) {}

    explicit BPriorityQueue(const Container &c, const Compare &cmp)
            : container_(c), cmp_(cmp) {
        heapify_();
    }

    explicit BPriorityQueue(Container &&c, const Compare &cmp)
            : container_(std::move(c)), cmp_(cmp) {
        heapify_();
    }

    // 通过一段可迭代的范围来创建容器
    template<typename InputIter>
    BPriorityQueue(InputIter beg, InputIter end,
                   const Container &c = Container(),
                   const Compare &cmp = Compare())
            : container_(c), cmp_(cmp) {
        while (beg != end) {
            container_.push_back(*beg);
            ++beg;
        }
        heapify_();
    }
// @}  // 各类构造函数


// @{  // 与容器容量相关的操作
public:

    // 返回容器是否为空
    bool empty() const {
        return container_.empty();
    }

    // 返回容器的大小
    SizeType size() const {
        return container_.size();
    }
// @}  // 与容器容量相关的操作


// @{  // 与元素访问相关的操作
public:
    // 返回容器首元素的 只读 索引
    const Tp &top() const {
        return container_.front();
    }
// @}  // 与元素访问相关的操作


// @{  // 向容器中添加元素的相关操作
private:
    /*
     * @brief 向上调整
     * @param idx 需要调整元素的索引
     *
     * idx的合法范围[0, container.size())，idx的合法
     * 性由调用者保证
     * */
    void adjust_up_(SizeType idx);

public:
    /*
     * @brief 向容器中添加元素
     * @param val 待添加的元素
     *
     * 添加元素后需要向上调整，时间复杂度为O(logn)
     * */
    void push(const Tp &val) {
        container_.push_back(val);
        adjust_up_(container_.size() - 1);
    }

    void push(Tp &&val) {
        container_.push_back(std::move(val));
        adjust_up_(container_.size() - 1);
    }

    /*
     * @brief 向容器尾部原位构造元素
     * @param args 转发到元素构造函数的参数
     *
     * 添加元素后需要向上调整，时间复杂度为O(logn)
     * */
    template<typename... Args>
    void emplace(Args &&... args) {
        container_.emplace_back(std::forward<Args>(args)...);
        adjust_up_(container_.size() - 1);
    }
// @}  // 向容器中添加元素的相关操作


// @{  // 在容器中删除元素的相关操作
private:
    /*
     * @brief 向下调整以使得容器满足堆的性质
     * @param idx 需要调整元素的索引
     * */
    void adjust_down_(SizeType idx);

public:
    /*
     * @brief 删除容器的首元素
     *
     * 删除元素后需要进行向下调整，时间复杂度为O(logn)
     * */
    void pop() {
        SizeType n = container_.size();
        std::swap(container_[0], container_[n - 1]);
        container_.pop_back();
        adjust_down_(0);
    }
// @}  // 在容器中删除元素的相关操作


    // 交换两个优先队列（BPriorityQueue）的内容
    void swap(BPriorityQueue &other) noexcept {
        std::swap(container_, other.container_);
        std::swap(cmp_, other.cmp_);
    }
};


// @{  // 类 BPriorityQueue 中声明的成员函数的实现
template<typename Tp, typename Container, typename Compare>
void BPriorityQueue<Tp, Container, Compare>::adjust_down_(SizeType idx) {
    SizeType len = container_.size();
    while (2 * idx + 1 < len) {
        // 在idx的两个子节点 2 * idx + 1 和 2 * idx + 2 中寻找用于交换的节点
        SizeType swap_idx = 2 * idx + 1;
        if (swap_idx + 1 < len
            && cmp_(container_[swap_idx], container_[swap_idx + 1])) {
            swap_idx++;
        }
        if (!cmp_(container_[idx], container_[swap_idx])) break;

        std::swap(container_[idx], container_[swap_idx]);
        idx = swap_idx;
    }
}

template<typename Tp, typename Container, typename Compare>
void BPriorityQueue<Tp, Container, Compare>::adjust_up_(SizeType idx) {
    while (idx > 0) {
        // idx父节点的索引
        SizeType p_idx = (idx - 1) / 2;
        if (cmp_(container_[idx], container_[p_idx])) {
            break;
        }
        std::swap(container_[idx], container_[p_idx]);
        idx = p_idx;
    }
}
// @}  // 类 BPriorityQueue 中声明的成员函数的实现

#endif //CPPBABYSTL_BABY_PRIORITYQUEUE_H
