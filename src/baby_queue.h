//
// Created by DELL on 2024/8/6.
//

#include "baby_deque.h"

#ifndef CPPBABYSTL_BABY_QUEUE_H
#define CPPBABYSTL_BABY_QUEUE_H


template<typename Tp, typename Container=BDeque<Tp>>
class BQueue {
public:
    using SizeType = typename Container::SizeType;

    template<typename Tp1, typename Container1>
    friend bool operator==(
            const BQueue<Tp1, Container1>& q1, const BQueue<Tp1, Container1>& q2);

private:
    Container container;

// @{  // 各类构造函数
public:

    // 默认构造函数
    BQueue() : container() {}

    // 拷贝构造函数
    BQueue(const BQueue &other)
            : container(other.container) {}

    // 移动构造函数
    BQueue(BQueue &&other) noexcept
            : container(std::move(other.container)) {}
// @}  // 各类构造函数


// @{  // 与容器容量相关的操作
public:

    // 判断容器是否为空
    bool empty() const {
        return container.empty();
    }

    // 返回当前容器的大小
    SizeType size() const {
        return container.size();
    }
// @}  // 与容器容量相关的操作


// @{  // 与元素访问相关的操作
public:

    // 返回容器首元素的 可读/可写 的引用
    Tp& front() {
        return container.front();
    }

    // 返回容器首元素的 可读 的引用
    const Tp& front() const {
        return container.front();
    }

    // 返回容器末尾元素的 可读/可写 的引用
    Tp& back() {
        return container.back();
    }

    // 返回容器末尾元素的 可读 的引用
    const Tp& back() const {
        return container.back();
    }
// @}  // 与元素访问相关的操作


// @{  // 向容器中添加元素的操作
public:
    /*
     * @brief 向容器中插入元素
     * @param val 需要插入的元素
     *
     * 队列仅支持在末尾插入元素
     * */
    void push(const Tp& val) {
        container.push_back(val);
    }

    void push(Tp&& val) {
        container.push_back(std::move(val));
    }

    /*
     * @brief 在容器中原位构造元素
     * @param args 转发到元素构造函数的参数
     * */
    template<typename... Args>
    void emplace(Args&&... args) {
        container.emplace_back(std::forward<Args>(args)...);
    }
// @}  // 向容器中添加元素的操作


// @{  // 在容器中删除元素的操作
public:
    // 删除末尾元素
    void pop() {
        container.pop_front();
    }
// @}  // 在容器中删除元素的操作
};


// BQueue 相关的非成员函数
template<typename Tp, typename Container>
bool operator==(const BQueue<Tp, Container>& q1, const BQueue<Tp, Container>& q2) {
    return q1.container == q2.container;
}

#endif //CPPBABYSTL_BABY_QUEUE_H
