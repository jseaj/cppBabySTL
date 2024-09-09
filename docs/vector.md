---
typora-root-url: figs
---

## `std::vector`简介及源码分布

​	`std::vector`是c++ STL提供的序列容器，与数组一样，其元素在内存中连续存储，因此在`std::vector`内对元素的访问操作能够在$O(1)$的时间复杂度内完成。但`std::vector`比数组更加灵活，因为`std::vector`的大小可以根据需求动态的增加，这使得`std::vector`非常适合需要在序列的尾部频繁的添加或移除元素的场景。

​	在标准库中，与`std::vector`实现相关的源代码主要分布在3个文件中：

- `vector`：使用`std::vector`时，需要导入的头文件`#include <vector>`实际就是该文件。在文件`vector`内，只是将与`std::vector`容器相关的所有头文件做了导入，不包含任何的具体实现。

- `bits/stl_vector.h`：该文件是`std::vector`在标准库内部的头文件，包含了`std::vector`类所有成员变量和函数的声明，以及一些简短函数的实现。
- `bits/vector.tcc`：`stl_vector.h`中声明的，但是实现起来较为复杂（代码量较大）的成员函数在该文件中实现。

## `std::vector`内部结构

​	在文件`bits/stl_vector.h`中，类`std::vector`的定义如下：

```c++
template <typename _Tp, typename _Alloc>
struct _Vector_base
{
    typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template rebind<_Tp>::other _Tp_alloc_type;
    typedef typename __gnu_cxx::__alloc_traits<_Tp_alloc_type>::pointer
        pointer;

    struct _Vector_impl
        : public _Tp_alloc_type
        {
            pointer _M_start;
            pointer _M_finish;
            pointer _M_end_of_storage;
            // 下面省略成员函数的定义
        };

    public:
    _Vector_impl _M_impl;
    // ...
};

template <typename _Tp, typename _Alloc = std::allocator<_Tp>>
class vector : protected _Vector_base<_Tp, _Alloc> {
    // 省略成员函数的定义
}
```

将上面的代码绘制为类图如下：

![](/vec-fig1.png)

从类图中可以清晰的观察到：类模版`vector<_Tp, _Alloc>`本身并未定义成员变量，而是通过继承的方式持有基类`_Vector_base<_Tp, _Alloc>`的类型为`_Vector_impl<_Tp>`成员变量`_M_impl`。因此，也可以简单的认为，在`std::vector`类中共有3个成员变量（实际上，`sizeof(std::vector<Tp>) = 24`正好是这3个成员变量的大小之和）：

| 变量名              | 类型                      | 含义                               |
| ------------------- | ------------------------- | ---------------------------------- |
| `_M_start`          | `_Tp*`（`_Tp`为模版类型） | 指向数组开始位置的指针             |
| `_M_finish`         | `_Tp*`                    | 指向数组末尾后一个位置的指针       |
| `_M_end_of_storage` | `_Tp*`                    | 指向数组结束位置的下一个位置的指针 |

> 在STL中，为什不将`std::vector`定义成下面这种简单的形式：
>
> ```c++
> template <typename _Tp, _Alloc = std::allocator<_Tp>>
> class vector {
> private:
>     _Tp *_M_start;
>     _Tp *_M_finish;
>     _Tp *_M_end_of_storage;
> }
> ```
>
> 而是通过如此复杂的方式去定义`std::vector`？

## `std::vector`部分重要操作及其实现原理

### 添加元素的相关操作

​	`std::vector`提供给用户，向容器中添加元素的函数有`insert`，`insert_range`，`push_back`，`emplace`和`emplace_back`，同时每个函数又有多个重载版本（更详细的介绍和函数的使用案例可参考https://zh.cppreference.com/w/cpp/container/vector）。向容器中添加元素时，如果`std::vector`维护的底层数组（`_M_start`指向的连续空间）容量不足，则会触发扩容操作。下面是一个案例，展示了向容量已满的`std::vector`容器中通过`push_back`插入新元素的流程：![vec-fig2](/vec-fig2.png)

案例主要有3个步骤：

1. 重新申请比旧数组大2倍（或1.5倍，取决于不同的编译器）的连续空间，图中申请的新数组大小为8。
2. 将旧数组的元素拷贝到新数组中，并将新元素2插入到新数组中的合适位置。
3. 释放掉旧数组，并修改`_M_start`，`_M_finish`和`_M_end_of_storage`的指向。

#### 浅拷贝 or 深拷贝

​	在上面的步骤中，有一个细节的问题值得考虑：在扩容的时候，从旧数组到新数组上的元素拷贝操作应该是浅拷贝还是深拷贝？当`std::vector`容器中存储的元素是基础数据类型（即模版类型`_Tp`为`int`，`char`等）时，考虑这个问题意义不大；而当`std::vector`容器中存储的元素是自定义的对象时，正确的使用数据的拷贝方式能够带来性能上的显著提升。在这种场景下，旧数组数据到新数组，实际上更像是“移动”而非“拷贝”，因此没有必要使用深拷贝。

​	那么，如何将旧数组中的数据通过浅拷贝的方式拷贝到新数组中呢？一种有效的方式是调用`memcpy`函数，`memcpy`能够将一个对象的数据复制到另一个对象中，并且不会触发该对象的拷贝构造函数，代码如下：

```c++
// 将旧数组 src 中的前 n 个数据“搬移”到新数组 dst 中
static void move_copy_(Tp *dst, Tp *src, SizeType n) {
    for (SizeType i = 0; i < n; i++) {
        std::memcpy(&dst[i], &src[i], sizeof(Tp));
        
        // “搬移”操作完成后，旧数组空间会被释放。释放旧数组的过程
        // 中会调用元素的析构函数，为了不影响到新数组，“搬移”结束
        // 后旧数组的数据应该变空
        std::memset(&src[i], 0, sizeof(Tp));
    }
}
```

> 注意：在`std::vector`的代码实现中，上述过程是通过调用`std::__uninitialized_move_if_noexcept_a`实现的，该函数被定义于文件`bits/stl_uninitialized.h`中，其内部也是调用了`memcpy`函数。

### 元素的删除操作

​	在`std::vector`中，我们可以通过调用`clear`，`erase`和`pop_back`等函数完成元素的删除操作。具体选择调用哪个函数、传递什么参数等问题可以参考https://zh.cppreference.com/w/cpp/container/vector。与添加元素不同，元素的删除操作不会导致`std::vector`容器的容量发生变化。元素的删除操作比较简单，但有一点需要注意：删除元素时应该调用该元素的析构函数，以确保正确的销毁掉该元素。

## `std::vector`的简单实现：`BVector`

`BVector`类是`std::vector`的简单实现，该类的定义如下：

```c++
template<typename Tp>
class BVector {
public:
    using SizeType = std::size_t;

private:
    Tp *start_{};           // 指向容器的首元素
    Tp *finish_{};          // 指向容器的已使用空间的结束位置
    Tp *end_of_storage_{};  // 指向容器的开辟空间的结束位置
}
```

在`BVector`类中实现了大部分`std::vector`提供给用户的接口，同时简化了代码的实现，完整代码请查看[`baby_vector.h`](../src/baby_vector.h)。
