//
// Created by DELL on 2024/8/6.
//

#include "baby_deque.h"

#ifndef CPPBABYSTL_BABY_STACK_H
#define CPPBABYSTL_BABY_STACK_H

template<typename Tp, typename Container=BDeque<Tp>>
class BStack {
public:
    using SizeType = typename Container::SizeType;

    template<typename Tp1, typename Container1>
    friend bool operator==(
            const BStack<Tp1, Container1>& st1, const BStack<Tp1, Container1>& st2);

private:
    Container container;

// @{  // 各类构造函数
public:

    // 默认构造函数
    BStack() : container() {}

    // 拷贝构造函数
    BStack(const BStack &other)
            : container(other.container) {}

    // 移动构造函数
    BStack(BStack &&other) noexcept
            : container(std::move(other.container)) {}
// @}  // 各类构造函数


// @{  // 与容器容量相关的操作
public:

    // 判断容器是否为空
    bool empty() const {
        return container.empty();
    }

    // 返回容器的大小
    SizeType size() {
        return container.size();
    }
// @}  // 与容器容量相关的操作


// @{  //  与元素访问相关的操作
public:

    // 返回栈顶的 可读/可写 引用
    Tp& top() {
        return container.back();
    }

    // 返回栈顶的 仅可读 引用
    const Tp& top() const {
        return container.back();
    }
// @}  //  与元素访问相关的操作


// @{  // 向容器中添加元素的相关操作
public:

    /*
     * @brief 向栈中压入新元素
     * @param val 需要压入栈中的元素值
     *
     * 栈只能在container的尾部添加元素
     * */
    void push(const Tp& val) {
        container.push_back(val);
    }

    void push(Tp&& val) {
        container.push_back(std::move(val));
    }

    /*
     * @brief 在栈顶原位构造元素
     * @param args 转发到元素构造函数的参数
     * */
    template<typename... Args>
    void emplace(Args&&... args) {
        container.emplace_back(std::forward<Args>(args)...);
    }
// @}  // 向容器中添加元素的相关操作


// @{  // 在容器中删除元素的相关操作
public:
    // 移除栈顶元素
    void pop() {
        container.pop_back();
    }
// @}  // 在容器中删除元素的相关操作
};


// @{  // 与类 BStack 相关的非成员函数
// 重载 == 运算符
template<typename Tp, typename Container>
bool operator==(
        const BStack<Tp, Container>& st1, const BStack<Tp, Container>& st2) {
    return st1.container == st2.container;
}
// @}  // 与类 BStack 相关的非成员函数

#endif //CPPBABYSTL_BABY_STACK_H
