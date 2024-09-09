//
// Created by DELL on 2024/7/15.
//

#include <cstddef>
#include <stdexcept>

#ifndef CPPBABYSTL_BABY_ARRAY_H
#define CPPBABYSTL_BABY_ARRAY_H


template <typename Tp, std::size_t Nm>
struct ArrayTrait {
    using Type = Tp[Nm];

    static constexpr Tp* ptr(const Type &t) {
        return static_cast<Tp*>(t);
    }
};

template <typename Tp>
struct ArrayTrait<Tp, 0> {
    using Type = struct {};

    static constexpr Tp* ptr(const Type &t) {
        return static_cast<Tp*>(nullptr);
    }
};

template<typename Tp, std::size_t Nm>
class BArray {
public:
    using SizeType = std::size_t;
    using ArrayType = ArrayTrait<Tp, Nm>;
    typename ArrayType::Type elems;

public:
    void fill(const Tp& val) {
        std::fill_n(this->data(), Nm, val);
    }


// @{  // 容量相关的操作
public:
    SizeType size() const { return Nm; }

    SizeType max_size() const { return Nm; }

    bool empty() const { return Nm == 0; }
// @}  // 容量相关的操作


// @{  // 与元素访问相关的操作
public:

    Tp& operator[](SizeType idx) {
        return this->data()[idx];
    }

    const Tp& operator[](SizeType idx) const {
        return this->data()[idx];
    }

    Tp& at(SizeType idx) {
        if (idx >= Nm) {
            throw std::out_of_range("array::at");
        }
        return this->data()[idx];
    }

    const Tp& at(SizeType idx) const {
        if (idx >= Nm) {
            throw std::out_of_range("array::at");
        }
        return this->data()[idx];
    }

    Tp& front() noexcept {
        return *this->data();
    }

    const Tp& front() const noexcept {
        return *this->data();
    }

    Tp& back() noexcept {
        return *(this->data() + Nm);
    }

    const Tp& back() const noexcept {
        return *(this->data() + Nm);
    }

    Tp* data() noexcept {
        return ArrayType::ptr(elems);
    }

    const Tp* data() const noexcept {
        return ArrayType::ptr(elems);
    }
// @}  // 与元素访问相关的操作


// @{  // 与迭代器相关的操作
public:
    Tp* begin() { return this->data(); }
    Tp* begin() const { return this->data(); }
    Tp* end() { return this->data() + Nm; }
    Tp* end() const { return this->data() + Nm; }
// @}  // 与迭代器相关的操作
};

#endif //CPPBABYSTL_BABY_ARRAY_H
