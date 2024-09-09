## `std::array`简介

​	`std::array`是对固定大小的数组的一个封装，使其在具有数组的性能的同时拥有标准容器的优点：

- 可获取大小。`std::array`数组支持通过`对象.size()`方法获取数组的大小。
- 支持赋值。数组无法通过`=`运算符进行拷贝赋值，而`std::array`可以通过`=`完成拷贝赋值。
- 数组名不是指针。在数组中，数组名也是数组的起始地址（c++隐式类型转换的结果），这容易给人带来困惑。而在`std::array`中，数组名和指针是两个不同的概念。

​	在使用`std::array`前，需要导入头文件：

```c++
#include <array>
```

在头文件`array`中包含了`std::array`的实现代码。

## 从0思考`std::array`的结构设计

​	在理解`std::array`的结构前，我们首先需要了解`std::array`的一些特有的“行为”：

- 定义一个`std::array`对象时需要同时指定存储元素的类型和数组的长度，例如：

  ```c++
  std::array<int, 4> arr;  // `int`表示数组元素的类型，`4`为数组的长度
  ```

- `std::array`封装的数组存储在栈空间而非堆空间。

- 在标准库中`std::array`是一个`struct`类型而非`class`。

- `std::array`允许定义一个长度为0的数组，例如：

  ```c++
  std::array<int, 0> arr;
  ```

  但是，在长度为0的`std::array`上调用`front()`和`back()`等访问元素的函数属于未定义的行为。

​	设计一个满足前3个“行为”的`array`很容易：

```C++
template<typename Tp, std::size_t Nm>
struct MyArray {
    Tp elems[Nm];    
};
```

但是，如果考虑第4个“行为”，即允许定义一个长度为0的`array`，上面的代码可能出现歧义。出现歧义是因为**柔性数组成员（flexible array）**，这属于是c语言的标准。因此，一种更好的实现方案是对`Nm = 0`的情况做一个偏特化：

```c++
template<typename Tp, std::size_t Nm>
struct MyArray {
    Tp elems[Nm];
    // ...成员函数
}

template<typename Tp>
struct MyArray<Tp, 0> {
    struct elems {};
    // ...成员函数
}
```

但是这种偏特化的实现方式也存在一个问题：所有的成员函数得写两遍，即使有的函数是一模一样的。是否存在一种更优的解决方案？C++ STL是将成员变量`elems`做一个萃取：

```C++
template<typename Tp, std::size_t Nm>
struct ArrayTrait {
    using Type = Tp[Nm];
    static constexpr Tp* ptr(const Type &t) {
        return static_cast<Tp*>(t);
    }
};

template<typename Tp>
struct ArrayTrait<Tp, 0> {
    using Type = struct {};
    static constexpr Tp* ptr(const Type &t) {
        return static_cast<Tp*>(nullptr);
    }
};

template<typename Tp, std::size_t Nm>
struct Array {
    using ArrayType = ArrayTrait<Tp, Nm>;
    typename ArrayType::Type elems;
};
```

STL中的代码要比上面的代码复杂一些：

```c++
template <typename _Tp, std::size_t _Nm>
  struct __array_traits
  {
    typedef _Tp _Type[_Nm];
    typedef __is_swappable<_Tp> _Is_swappable;
    typedef __is_nothrow_swappable<_Tp> _Is_nothrow_swappable;

    static constexpr _Tp &
    _S_ref(const _Type &__t, std::size_t __n) noexcept
    {
      return const_cast<_Tp &>(__t[__n]);
    }

    static constexpr _Tp *
    _S_ptr(const _Type &__t) noexcept
    {
      return const_cast<_Tp *>(__t);
    }
  };

  template <typename _Tp>
  struct __array_traits<_Tp, 0>
  {
    struct _Type
    {
    };
    typedef true_type _Is_swappable;
    typedef true_type _Is_nothrow_swappable;

    static constexpr _Tp &
    _S_ref(const _Type &, std::size_t) noexcept
    {
      return *static_cast<_Tp *>(nullptr);
    }

    static constexpr _Tp *
    _S_ptr(const _Type &) noexcept
    {
      return nullptr;
    }
  };
template <typename _Tp, std::size_t _Nm>
struct array
{
    typedef _Tp value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef value_type *iterator;
    typedef const value_type *const_iterator;
    typedef std::size_t SizeType;
    typedef std::ptrdiff_t difference_type;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Support for zero-sized arrays mandatory.
    typedef _GLIBCXX_STD_C::__array_traits<_Tp, _Nm> _AT_Type;
    typename _AT_Type::_Type _M_elems;
}
```

## `std::array`的简单实现：`BArray`

`BArray`的实现详见[`baby_array.h`](../src/baby_array.h)。
