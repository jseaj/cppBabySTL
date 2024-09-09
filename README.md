# cppBabySTL简介

cppBabySTL是对c++ STL源代码的学习和仿写，在本项目中包含了**解析STL源码的详细笔记**和**可读性较高的代码仿写**。本项目之所以称之为“baby”，是因为解析源码和仿写的过程中做了如下简化：

- 只包含容器和简易的迭代器。STL 分为多个组件，包括容器（Containers）、迭代器（Iterators）、算法（Algorithms）、函数对象（Function Objects）、分配器（allocators）和适配器（Adapters）等。而cppBabySTL仅针对容器，包括但不限于`string`，`array`，`vector`，`deque`和`map`等。此外，在仿写的代码中同样包含迭代器，但该迭代器仅仅是为了使实现的容器支持范围for循环的语法。
- 仿写的容器不支持分配器。在STL中，分配器用于管理容器的内存空间。但在本项目中，我们采用`new`/`delete`对内存进行手动管理。
- 仅提供c++17及以上的语法支持。

# 运行项目

本项目通过`cmake`进行构建，`./main.cpp`为入口文件，但仅用作测试。运行`./main.cpp`的方式如下：

```shell
$ cmake ./
$ make
```

# 如何学习本项目

在学习本项目前，首先需要解决一个问题，即：在哪儿阅读c++ STL源代码？主要有以下两种方式：

1. 通过编译器自带的源代码进行学习。c++的编译器（如GCC、MinGW和MSVC等）都是自带源代码，但不同的编译器对STL的实现并不完全相同。
2. 在官方的github上的镜像阅读源码。GNU/GCC中的STL镜像地址为：https://github.com/gcc-mirror/gcc/tree/master/libstdc%2B%2B-v3/include/bits，MSVC的官方镜像：https://github.com/microsoft/STL/tree/main。

个人推荐使用方式1去阅读STL的源代码，因为方式1可以debug。

## 目录

- 阅读源码前的准备工作
  - STL源码的命名风格 VS 本项目的命名风格：TODO
  - `typename`，`typedef`和`using`在STL源码中的作用：TODO
- [`std::string`源码阅读笔记](./docs/string.md)，[`std::string`仿写代码](./src/baby_string.md)
- [`std::array`源码阅读笔记](./docs/array.md)，[`std::array`仿写代码](./src/baby_array.h)
- [`std::vector`源码阅读笔记](./docs/vector.md)，[`std::vector`仿写代码](./src/baby_vector.h)
- [`std::forwardlist`源码阅读笔记](./docs/forwardlist.md)，[`std::forwardlist`仿写代码](./src/baby_forwardlist.h)
- `std::list`源码阅读笔记：TODO，[`std::list`仿写代码](./src/baby_list.h)
- `std::deque`源码阅读笔记：TODO，[`std::deque`仿写代码](./src/baby_deque.h)
- `std::stack`源码阅读笔记：TODO，[`std::stack`仿写代码](./src/baby_stack.h)
- `std::queue`源码阅读笔记：TODO，[`std::queue`仿写代码](./src/baby_queue.h)
- `std::priority_queue`源码阅读笔记：TODO，[`std::priority_queue`仿写代码](./src/baby_priorityqueue.h)
- STL中的红黑树：TODO，[红黑树的代码实现](./src/rb_tree.h)
- `std::map`源码阅读笔记：TODO，[`std::map`仿写代码](./src/baby_map.h)
- `std::multimap`源码阅读笔记：TODO，[`std::multimap`仿写代码](./src/baby_multimap.h)
- `std::set`源码阅读笔记：TODO，[`std::set`仿写代码](./src/baby_set.h)
- `std::multiset`源码阅读笔记：TODO，[`std::multiset`仿写代码](./src/baby_multiset.h)

# 主要参考资料

1. 侯捷：STL源码剖析

2. OI WIKI：https://oi-wiki.org/

3. 一些与STL相关的博文：https://cloud.tencent.com/developer/column/91626
4. c++参考手册中的容器库：https://zh.cppreference.com/w/cpp/container
