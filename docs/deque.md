---
typora-root-url: figs
---

## std::deque简介

​	`std::deque` 是 C++ STL中提供的一种顺序容器。使用`std::deque`时需要首先`include`头文件`deque`：

```c++
#include <deque>
```

头文件`deque`中只是将与容器`std::deque`相关文件做了导入，并不包含具体的实现。而`std::deque`容器实现的代码主要在文件`stl_deque.h`和`deque.tcc`中：

- `stl_deque.h`：位于文件夹`xxx/mingw64/lib/gcc/x86_64-w64-mingw32/8.1.0/include/c++/bits`其中`xxx`表示可变动的位置。`stl_deque.h`文件包含了`std::deque`容器所有成员变量和成员函数的声明（包括迭代器），同时还包含了部分简单的成员函数的实现。
- `deque.tcc`：该文件与`stl_deque.h`位于同一级目录，实现了在`stl_deque.h`声明了但没有实现的成员函数。

## std::deque结构

​	`std::deque`允许在两端快速地进行插入和删除操作。与`std::vector`相比，`std::deque`在两端的插入和删除操作具有更高的效率，因为`std::vector`在头部插入元素时需要移动所有现有元素，因此时间复杂度为O(n)。相比于`std::list`，`std::deque`提供了对元素的随机访问能力。`std::list`也支持在两端快速插入和删除，但访问特定位置的元素需要从头开始遍历，这使得其访问时间复杂度为O(n)。

​	然而，提到双端队列，我们容易错误地将`std::deque`想象为类似于下图所示的一种简单的线性结构：

![deq-fig2](/deq-fig2.png)

但实际上，`std::deque`的内部实现要复杂得多。例如我们有一个`std::deque`，它存储了整数序列`[1, 2, 3, 4, 5, 6]`。那么在内存中，这个示例的结构如下图所示：

![deq-fig1](/deq-fig1.png)

在上图中，共有4个重要的变量：`_M_map`、`_M_map_size`、`start_`和`finish_`：

- `_M_map`：大小为`_M_map_size`的指针数组。`_M_map`也被称为**中控器**，主要用于维护`std::deque`整体连续的假象，并提供随机存取的接口。`_M_map`数组中存储的非空元素也被称之为**节点（node）**，每个节点指向一块大小固定的连续空间（即数组，也称**缓冲区**）。注意，此处的“大小固定”不是指数组的元素数量是固定的，而是指内存块的大小是固定的，如在`std::deque`中被设置为512 bytes，该值在编译时确定并且不允许用户运行时修改。
- `_M_map_size`：指针数组`_M_map`的长度。需要注意的是，`_M_map_size`有一个最小长度，其值为8。如示例所示，存储整数序列`[1, 2, 3, 4, 5, 6]`的`deque`只需要将`_M_map_size`设置为2即可，但由于限制了`_M_map_size`最小值为8，故示例中`_M_map_size=8`。
- `start_`：指向`std::deque`中第一个元素的迭代器。
- `finish_`：迭代器，其指向`std::deque`中最后一个元素的下一个位置。