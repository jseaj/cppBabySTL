//
// Created by DELL on 2024/7/10.
//

#include <cstddef>
#include <memory>
#include <iostream>
#include <initializer_list>
#include <cstring>

#ifndef CPPBABYSTL_BABY_VECTOR_H
#define CPPBABYSTL_BABY_VECTOR_H

template<typename Tp>
class BVector {
public:
    using SizeType = std::size_t;

private:
    Tp *start_{};           // 指向容器的首元素
    Tp *finish_{};          // 指向容器的已使用空间的结束位置
    Tp *end_of_storage_{};  // 指向容器的开辟空间的结束位置

private:
    /*
     * @brief 将数组 src 中前 n 个元素复制到数组 dst 中，同时将 src 中前 n 个元素置为空
     * @param dst 指向目标数组的指针
     * @param src 指向源数组的指针
     * @param n   需要复制的元素数量
     *
     * 不进行越界检查，调用者需保证参数的合法性
     * */
    static void move_copy_(Tp *dst, Tp *src, SizeType n) {
        for (SizeType i = 0; i < n; i++) {
            std::memcpy(&dst[i], &src[i], sizeof(Tp));
            std::memset(&src[i], 0, sizeof(Tp));
        }
    }

    /*
     * @brief 将 [beg, end) 内的所有元素向右移动 cnt 个单位
     * @param beg 开始的位置
     * @param end 结束位置的下一个位置
     * @param cnt 需要移动的数目
     *
     * 调用该函数需要保证`数组`的空间 >= cnt + size()，此外还需确保
     * 被覆盖掉的位置的数据已经无效或已被正确析构
     * */
    static void move_right_(Tp *beg, Tp *end, SizeType cnt) {
        for (auto it = end - 1; it >= beg; --it) {
            // 此处使用memcpy是不想触发对象的拷贝构造函数，减少不必要的额外开销
            // 类似对象的移动语义，但如果Tp类没有定义移动语义，仍然会使用拷贝构造
            std::memcpy(it + cnt, it, sizeof(Tp));
            std::memset(it, 0, sizeof(Tp));
        }
    }

    /*
     * @brief 将 [beg, end) 内所有元素向左移动 cnt 个单位
     * @param beg 开始的位置
     * @param end 结束位置的下一个位置
     * @param cnt 需要移动的数目
     *
     * 调用该函数需要保证`数组`的空间 >= cnt + size()，此外还需确保
     * 被覆盖掉的位置的数据已经无效或已被正确析构
     * */
    static void move_left_(Tp *beg, Tp *end, SizeType cnt) {
        for (auto it = beg; it != end; ++it) {
            std::memcpy(it - cnt, it, sizeof(Tp));
            std::memset(it, 0, sizeof(Tp));
        }
    }


// @{  // 各种构造函数 / 析构函数
private:
    /*
     * @brief 动态调整内存容量
     * @param new_capacity 新请求的容量大小
     * @param old_capacity 当前容器的原始容量大小
     *
     * 注意：实际分配的新空间大小并不一定是 new_capacity
     * */
    void adjust_capacity_(SizeType new_capacity, SizeType old_capacity);

    /*
     * @brief 将数组 src 中前 n 个元素复制到数组 dst 中
     * @param dst 指向目标数组的指针
     * @param src 指向源数组的指针
     * @param n   需要复制的元素数量
     *
     * 不进行越界检查，调用者需保证参数的合法性
     * */
    static void copy_(Tp *dst, const Tp *src, SizeType n) {
        for (SizeType i = 0; i < n; i++) {
            dst[i] = src[i];
        }
    }

    // 释放BVector容器底层数组的内存空间
    void destroy_() {
        delete[] start_;
        start_ = finish_ = end_of_storage_ = nullptr;
    }

public:

    // 默认构造函数
    BVector() = default;

    // 拷贝构造函数
    BVector(const BVector &other) {
        adjust_capacity_(other.size(), 0);
        copy_(start_, other.start_, other.size());
        finish_ = start_ + other.size();
    }

    // 移动构造函数
    BVector(BVector &&other) noexcept {
        start_ = other.start_;
        finish_ = other.finish_;
        end_of_storage_ = other.end_of_storage_;

        other.start_ = other.finish_ = other.end_of_storage_ = nullptr;
    }

    // 创建一个长度为 n，值全为默认值的容器
    explicit BVector(SizeType n) {
        // 开辟存储空间
        adjust_capacity_(n, 0);
        finish_ = start_ + n;
    }

    // 创建一个长度为 n，值全为 val 的容器
    explicit BVector(SizeType n, const Tp &val) {
        // 开辟存储空间
        adjust_capacity_(n, 0);

        for (SizeType i = 0; i < n; i++) {
            // 调用 Tp 类型的拷贝构造函数
            new(start_ + i) Tp(val);
        }
        finish_ = start_ + n;
    }

    // 使用初始化列表构造容器
    BVector(std::initializer_list<Tp> init_list) {
        adjust_capacity_(init_list.size(), 0);
        copy_(start_, init_list.begin(), init_list.size());
        finish_ = start_ + init_list.size();
    }

    // 析构函数
    ~BVector() { destroy_(); }
// @}  // 各种构造函数 / 析构函数


// @{  // 与赋值相关的操作
private:

    /*
     * @brief 销毁指定范围内对象的实例
     * @param beg 指向需要销毁对象的起始位置的指针
     * @param end 指向需要销毁对象的结束位置的下一个位置的指针
     *
     * 该函数不执行内存释放，只负责调用析构函数，同时调用者必须
     * 保证[beg, end)位置的合法
     * */
    static void destruct_range_(Tp *beg, Tp *end) {
        // 基本数据类型无需调用析构函数
        if (std::is_fundamental_v<Tp>) return;

        for (auto it = beg; it != end; ++it) {
            it->~Tp();
        }
    }

    /*
     * @brief 将迭代器范围 [beg, end) 所表示的元素赋值给当前容器
     * @param beg 迭代器，指向待赋值数据的第一个元素
     * @param end 迭代器，指向待赋值数据的最后一个元素的下一个位置
     *
     * 该函数供赋值运算操作所调用，不提供给用户
     * */
    template<typename InputIter>
    void assign_(InputIter beg, InputIter end);

public:

    // 拷贝赋值函数
    BVector &operator=(const BVector &other) {
        if (this == &other) return *this;  // 防止自赋值
        assign_(other.begin(), other.end());
        return *this;
    }

    // 移动赋值函数
    BVector &operator=(BVector &&other) noexcept {
        start_ = other.start_;
        finish_ = other.finish_;
        end_of_storage_ = other.end_of_storage_;

        other.start_ = other.finish_ = other.end_of_storage_ = nullptr;
        return *this;
    }

    // 通过初始化列表进行赋值
    BVector &operator=(std::initializer_list<Tp> init_list) {
        assign_(init_list.begin(), init_list.end());
        return *this;
    }
// @}  // 与赋值相关的操作


// @{  // 与容量相关的操作
public:

    // 检查容器是否为空
    bool empty() const {
        return start_ == finish_;
    }

    // 返回容器的元素数目
    SizeType size() const {
        return SizeType(finish_ - start_);
    }

    // 返回当前存储空间能够容纳的元素数
    SizeType capacity() const {
        return SizeType(end_of_storage_ - start_);
    }
// @}  // 与容量相关的操作


// @{  // 与元素访问相关的操作
public:

    // 返回索引为 idx 的元素引用，不进行边界检查
    const Tp &operator[](SizeType idx) const {
        return *(start_ + idx);
    }

    // 返回索引为 idx 的元素引用，不进行边界检查
    Tp &operator[](SizeType idx) {
        return *(start_ + idx);
    }

    // 返回索引为 idx 的元素引用，带边界检查
    const Tp &at(SizeType idx) const {
        if (idx >= this->size()) {
            throw std::out_of_range("BVector::at");
        }
        return *(start_ + idx);
    }

    // 返回索引为 idx 的元素引用，带边界检查
    Tp &at(SizeType idx) {
        if (idx >= this->size()) {
            throw std::out_of_range("BVector::at");
        }
        return *(start_ + idx);
    }

    // 返回容器首元素的引用，在空容器上调用front()属于未定义的行为
    const Tp &front() const {
        return operator[](0);
    }

    // 返回容器首元素的引用，在空容器上调用front()属于未定义的行为
    Tp &front() {
        return operator[](0);
    }

    // 返回容器中最后一个元素的引用，在空容器上调用back()属于未定义的行为
    const Tp &back() const {
        return operator[](this->size() - 1);
    }

    // 返回容器中最后一个元素的引用，在空容器上调用back()属于未定义的行为
    Tp &back() {
        return operator[](this->size() - 1);
    }

    /*
     * @brief 返回指向作为元素存储工作的底层数组的指针
     *
     * 返回的指针使得范围 [data(), data() + size())始终是有效范围，
     * 即使容器为空（此时 data() 不可解引用）
     * */
    const Tp *data() const { return start_; }

    /*
     * @brief 返回指向作为元素存储工作的底层数组的指针
     *
     * 返回的指针使得范围 [data(), data() + size())始终是有效范围，
     * 即使容器为空（此时 data()不可解引用）
     * */
    Tp *data() { return start_; }
// @}  // 与元素访问相关的操作


// @{  // 向容器中添加元素的相关操作
public:

    /*
     * @brief 插入多个元素元素到容器的指定位置
     * @param idx 插入元素的起始索引，[0, size()]
     * @param cnt 要插入的元素数量
     * @param val 要插入的元素值
     * */
    void insert(SizeType idx, SizeType cnt, const Tp &val);

    /*
     * @brief 插入单个元素到指定位置
     * @param idx 插入元素的起始索引，[0, size()]
     * @param val 插入元素的值
     * */
    void insert(SizeType idx, const Tp &val) {
        this->insert(idx, 1, val);
    }

    /*
     * @brief 插入单个元素到指定位置
     * @param idx 插入元素的起始索引，[0, size()]
     * @param val 插入元素的值
     * */
    void insert(SizeType idx, Tp &&val) {
        this->insert(idx, val);
    }

    /*
     * @brief 添加新元素到容器尾
     * @param args 转发到元素构造函数的实参
     * */
    template<typename... Args>
    void emplace_back(Args &&... args) {
        if (this->size() + 1 > this->capacity()) {
            adjust_capacity_(this->size() + 1, this->capacity());
        }

        // placement new 构造对象
        new(finish_) Tp(std::forward<Args>(args)...);
        ++finish_;
    }

    // 在容器尾部追加 val
    void push_back(const Tp &val) {
        this->insert(this->size(), 1, val);
    }

    // 在容器尾部追加 val
    void push_back(Tp &&val) {
        this->insert(this->size(), 1, val);
    }
// @}  // 向容器中添加元素的相关操作


// @{  // 在容器中删除元素的相关操作
public:

    // 清空容器：调用后 size() 为 0，但 capacity() 不变
    void clear() {
        // 在[start_, finish_)上调用对象的析构函数
        destruct_range_(start_, finish_);
        finish_ = start_;
    }

    /*
     * @brief 重设容器大小以容纳 cnt 个元素，在 cnt == size() 时不做任何操作
     * @param cnt 容器的大小（注意不是capacity()）
     * @param val 用以初始化新元素的值
     *
     * 当容器的 size() == cnt 时，不进行任何操作
     * 当容器的 size() > cnt 时，析构掉 [cnt, size())内所有的对象
     * 当容器的 size() < cnt 时，将 [size(), cnt)内所有元素设为 val
     * */
    void resize(SizeType cnt, const Tp &val);

    void resize(SizeType _cnt) {
        this->resize(_cnt, Tp{});
    }

    /*
     * @brief 从容器中移除[beg, end)的元素
     * @param beg 移除范围首地址
     * @param end 移除范围尾地址
     *
     * 该函数不抛出异常，除非Tp类型的赋值运算出现异常
     * */
    void erase(SizeType beg, SizeType end) {
        if (beg >= this->size()) return;
        if (beg >= end) return;
        end = std::min(end, this->size());

        // [beg, end)内的元素调用析构函数
        destruct_range_(start_ + beg, start_ + end);

        // [end, size())上的所有元素左移 end - beg 个单位
        move_left_(start_ + end, start_ + this->size(), end - beg);
        finish_ -= (end - beg);
    }

    // 从容器中移除索引为 idx 的元素
    void erase(SizeType idx) {
        this->erase(idx, idx + 1);
    }

    // 移除容器末尾元素，空容器上调用该函数不进行任何操作
    void pop_back() {
        this->erase(this->size() - 1);
    }
// @}  // 在容器中删除元素的相关操作


// @}  // 迭代器相关的操作
public:
    // 底层存储结构为数组，直接使用 Tp* 做迭代器即可

    Tp *begin() { return start_; }
    Tp *begin() const { return start_; }
    Tp *end() { return finish_; }
    Tp *end() const { return finish_; }
// @}  // 迭代器相关的操作
};


// 重载 == 运算赋
template<typename Tp>
bool operator==(const BVector<Tp> &x, const BVector<Tp> &y);


/* 类 BVector 中声明但没有实现的成员函数 */
template<typename Tp>
void BVector<Tp>::resize(BVector::SizeType cnt, const Tp &val) {
    if (cnt == this->size()) return;

    if (cnt > this->capacity()) {
        adjust_capacity_(cnt, this->capacity());
    }

    if (cnt < this->size()) {
        // 析构掉[cnt, size())内的所有对象
        destruct_range_(start_ + cnt, finish_);
    } else {
        for (SizeType i = this->size(); i < cnt; i++) {
            new (start_ + i) Tp(val);
        }
    }
    finish_ = start_ + cnt;
}

template<typename Tp>
void BVector<Tp>::insert(
        BVector::SizeType idx, BVector::SizeType cnt, const Tp &val) {
    if (idx > this->size()) {
        throw std::out_of_range("BVector::insert");
    }

    SizeType len = this->size() + cnt;
    if (len > this->capacity()) {
        // 重新开辟空间
        adjust_capacity_(len, this->capacity());
    }

    if (idx < this->size()) {
        // [idx, idx + size())内所有元素后移_cnt个位置
        move_right_(start_ + idx, start_ + idx + this->size(), cnt);
    }
    for (SizeType i = 0; i < cnt; i++) {
        new (start_ + idx + i) Tp(val);
    }

    // 修改尾指针指向
    finish_ += cnt;
}

template<typename Tp>
template<typename InputIter>
void BVector<Tp>::assign_(InputIter beg, InputIter end) {
    SizeType len = end - beg;
    if (len > this->capacity()) {
        adjust_capacity_(len, this->capacity());
    } else if (this->size() > len) {
        // 将多余的元素析构掉
        destruct_range_(start_ + len, finish_);
    }

    // 复制内容
    copy_(start_, beg, len);
    finish_ = start_ + len;
}


template<typename Tp>
void BVector<Tp>::adjust_capacity_(
        BVector::SizeType new_capacity, BVector::SizeType old_capacity) {
    if (new_capacity < 2 * old_capacity) {
        new_capacity = 2 * old_capacity;
    }
    SizeType len = this->size();

    auto tmp = new Tp[new_capacity]{};
    move_copy_(tmp, start_, len);
    destroy_();  // 释放原来的内存

    start_ = tmp;
    finish_ = start_ + len;
    end_of_storage_ = start_ + new_capacity;
}

template<typename Tp>
bool operator==(const BVector<Tp> &x, const BVector<Tp> &y) {
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

#endif //CPPBABYSTL_BABY_VECTOR_H
