## `std::string`简介

​	`std::string`是C++标准库提供的字符串类之一（除了`std::string`之外还有`std::wstring`、`std::u8string`等）。相比于c风格字符串（`char*`类型），`std::string`提供了更丰富的字符串操作接口。

​	`std::string`的源代码主要分布在3个文件中：`string`、`bits/basic_string.h`和`bits/basic_string.tcc`：

- `string`：使用`std::string`前需要导入该头文件，即`#include <string>`。但在文件`string`内，并不包含具体实现的代码，仅仅是将与`std::string`相关的头文件进行了导入。
- `bits/basic_string.h`：在该文件中定义并实现了类模板`std::basic_string`（`std::string`是`std::basic_string`的特化），并对`std::basic_string`中大部分较为简短的成员函数进行了实现。
- `bits/basic_string.tcc`：该文件内包含了未在`bits/basic_string.h`实现的较为复杂的成员函数的代码实现。

## `std::string`结构

​	前文提到`std::string`是类模版`std::basic_string`的特化，因此想要理解`std::string`的结构和实现原理，直接查看`std::basic_string`。在文件`bits/basic_string.h`中，类`std::basic_string`的定义如下：

```c++
template <typename _CharT, typename _Traits, typename _Alloc>
class basic_string
{
    // ...
    ///  Value returned by various member functions when they fail.
    static const SizeType npos = static_cast<SizeType>(-1);

    private:
    // Use empty-base optimization: http://www.cantrip.org/emptyopt.html
    struct _Alloc_hider : allocator_type // TODO check __is_final
    {
        _Alloc_hider(pointer __dat, const _Alloc &__a)
            : allocator_type(__a), _M_p(__dat) {}

        _Alloc_hider(pointer __dat, _Alloc &&__a = _Alloc())
            : allocator_type(std::move(__a)), _M_p(__dat) {}

        pointer _M_p; // The actual data.
    };

    _Alloc_hider _M_dataplus;
    SizeType _M_string_length;

    enum
    {
        _S_local_capacity = 15 / sizeof(_CharT)
    };

    union
    {
        _CharT _M_local_buf[_S_local_capacity + 1];
        SizeType _M_allocated_capacity;
    };
    // ...
}
```

实际上，上面的代码等价于（还原类型别名、去掉`empty-base optimization`和修改变量的命名方式）：

```c++
class basic_string
{
    // ...
public:    
    ///  Value returned by various member functions when they fail.
    static const std::size_t npos = static_cast<std::size_t>(-1);

private:
    char *p;                    // c风格字符串数组
    std::size_t string_length;  // 字符串的长度

    enum
    {
        local_capacity = 15 / sizeof(char)
    };

    union
    {
        char local_buf[local_capacity + 1];
        std::size_t allocated_capacity;
    };
    // ...
}
```

从代码中可以看出，`std::basic_string`类共有4个成员变量：

| 变量名               | 类型                               | 描述                                  |
| -------------------- | ---------------------------------- | ------------------------------------- |
| `p`                  | `char*`                            | c风格字符串数组，存储真实的字符串数据 |
| `string_length`      | `std::size_t`                      | 字符串长度，不包括结尾的`'\0'`        |
| `local_buf`          | 在`union`中定义的`char[]`类型      | 在栈上分配的空间，用于小字符串的优化  |
| `allocated_capacity` | 在`union`中定义的`std::size_t`类型 | 记录字符串`p`的空间大小               |

变量`p`、`string_length`和`allocated_capacity`的含义很容易理解，而`local_buf`主要用于对小字符串进行优化。这种优化的基本思路是：操作存储在栈上的字符串比操作存储在堆上的字符串更快，但如果把所有字符存储在栈上，空间不够，因此考虑将小字符串存储在栈上。上述思路具体实现的策略如下：

- 当字符串长度 $\leq$ `local_capacity = 15`时：字符串将存储在栈上开辟的空间，也就是`local_buf`中。
- 当字符串长度 $>$ `local_capacity = 15`时：字符串将存储在堆上开辟的空间，也就是`p`中。此时，`local_buf`表示的空间将用来表示堆上分配空间的大小，也即`allocated_capacity`。

> 需要注意的是：并不是当字符串的长度 $\leq$ `local_capacity = 15`，该串就一定存储在栈上。参考下面这个例子：
>
> ```c++
> string str(32, 'a');
> for (int i = 0; i < 30; i++) {
>     str.pop_back();
> }
> cout << "size: " << str.size() << endl;          // size: 2
> cout << "capacity: " << str.capacity() << endl;  // capacity: 32
> ```

​	将`local_buf`和`allocated_capacity`放入到`union`类型中是为了节省空间，通过`sizeof`关键字计算`std::string`的大小为32：

```
sizeof(string) = sizeof(char*) + sizeof(size_t) + max(sizeof(char[16]), sizeof(size_t))
               = 8 + 8 + 16
               = 32
```

**Q1：为什么将`local_capacity`声明为enum类型？**

使用enum声明的变量是编译期常量（Compile-time constants），而在类中，数组的长度必须是编译期常量，例如下面这段代码会报错：

```c++
class Demo {
    const int N = 10;
    int arr[N];  // 报错，N必须是编译期常量
}
```

而下面的代码则能正确被编译：

```c++
class Demo {
    enum {N = 10};
    int arr[N];  // 正确，N为编译期常量
}
```

在c++11及以后得标准中，也可以使用下面的方式：

```c++
enum { local_capacity = 15 / sizeof(char) };

// constexpr在c++11及之后的版本才被支持
static constexpr std::size_t local_capacity = 15 / sizeof(char);
```

总之，`enum`能够声明一个编译器常量，并且能够兼容c++的各种版本，更具通用性。

## `std::string`常见操作的实现原理

### 向`std::string`中添加数据

​	不同于c风格字符串，`std::string`所表示的字符串是动态的，这意味着我们可以不断地向字符串中添加字符，直到达到容量上限（可以通过`对象.max_size()`方法查看）。但从`std::basic_string`的源代码可知，真实数据还是存储在内部的c风格字符串（`p`）中的，那“动态”性是如何保证的？难道一开始就为`p`申请了`max_size()`大小的空间？答案是否定的，实际上与`std::vector`一样，**扩容机制**保证了`std::string`字符串的“动态性”，使得在用户看来`std::string`字符串的容量是无限的。

​	下面的例子展示了，向`std::string`中添加元素时的过程：![str-fig1](F:\cppCode\cppBabySTL\docs\figs\str-fig1.png)

在上面的例子中，在索引为2的位置处插入`s="123"`，但此时`std::string`内部维护的`p`串空间为5，不足以容纳3个字符，因此触发扩容机制：

1. 开辟一个大小为`p`串空间2倍的新空间；
2. 将旧空间的内容复制到新空间中；
3. 在新空间中插入内容；
4. 释放掉旧空间，并将`p`重新指向新空间。

> 注意：
>
> - 在`std::string`中，`p`所指向的空间大小至少为`local_capacity = 15`，图例所示的情况是不可能出现的；
> - 在代码的实现过程中还需要注意很多细节，如插入位置是否越界、插入串`s`可能是`p`的子串等，这些问题均未在图例中展示。

### 在`std::string`中删除数据

​	删除数据的操作比较简单，只需要将待删除位置之后的数据前移，覆盖掉需要删除位置的数据即可，时间复杂度为$O(n)$，其中$n$表示字符串的长度。需要注意的是，删除操作不会导致“缩容”，这也解释了前文提到的：为什么长度小于等于`local_capacity`的字符串，其数据并不一定是存储在栈上。

## 实现一个简单的`std::string`类：`BString`

​	`BString`类的定义如下：

```c++
class BString {
public:
    using SizeType = std::size_t;

    // 一些成员函数失败时返回的值，如 find
    static const SizeType npos = static_cast<SizeType>(-1);

private:
    char *ptr_;
    SizeType str_len_{};  // 字符串长度
    enum {
        kLocalCapacity = 15
    };  // 小字符串的长度

    union {
        char local_buf_[kLocalCapacity + 1]{};

        // 在堆上分配的空间，实际空间为 allocated_capacity_ + 1
        // 其中1表示最后一位存储'\0'
        SizeType allocated_capacity_;
    };
}
```

更多的实现细节可参考