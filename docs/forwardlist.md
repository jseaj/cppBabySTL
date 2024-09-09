---
typora-root-url: figs
---

## `std::forward_list`简介

​	`std::forward_list` 是 C++ 标准库中的一个单向链表容器。与 `std::list` 不同（`std::list`是双向循环链表），`std::forward_list` 只支持向前遍历，在只需要简单的插入和删除场景中`std::forward_list`表现得比`std::list`更加高效。

​	使用`std::forward_list`时需要首先包含头文件`forward_list`：

```c++
#include <forward_list>
```

然而在头文件`forward_list`中只是将与容器`std::forward_list`相关文件做了导入，并不包含具体的实现。而`std::forward_list`容器实现的代码主要在文件`bits/forward_list.h`和`bits/forward_list.tcc`中：

- `bits/forward_list.h`：`forward_list.h`文件包含了`std::forward_list`容器所有成员变量和成员函数的声明，同时还包含了部分简单的成员函数的实现。
- `bits/forward_list.tcc`：在该文件中，实现了在`forward_list.h`声明了但没有实现的成员函数。

## `std::forward_list`结构

​	`std::forward_list`是一个单向链表，链表的节点共有两种类型：`_Fwd_list_node_base`和`_Fwd_list_node<_Tp>`：

- `_Fwd_list_node_base`：只存储下一个节点的指针，并不存储数据。在`std::forward_list`中用作头结点。
- `_Fwd_list_node<_Tp>`：既存储下一个节点的指针，也存储类型为`_Tp`的数据。在`std::forward_list`中用于存储数据，同时`_Fwd_list_node<_Tp>`是`_Fwd_list_node_base`的派生类。

![fwd-fig2](/fwd-fig2.png)

上述两个节点类的定义如下：

```c++
struct _Fwd_list_node_base
{
    // ... 成员函数
    _Fwd_list_node_base *_M_next = nullptr;
};

template <typename _Tp>
struct _Fwd_list_node
      : public _Fwd_list_node_base
{
    __gnu_cxx::__aligned_buffer<_Tp> _M_storage;
    // ... 成员函数
};
```

`std::forward_list`的类图如下：

![fwd-fig1](/fwd-fig1.png)

从类图中可以看出，类`forward_list<_Tp>`是通过继承的方式获得数据，本身只是向用户提供了`std::forward_list`的接口。实际上，真正对数据的操作是由`_Fwd_list_base<_Tp>`和`_Fwd_list_impl`两个类完成的，几乎所有的标准库提供的容器都是采用这种方式实现的。

​	类`_Fwd_list_node_base`是单链表的头节点类，在该类中只有指向下一节点的指针`next`，并不存储数据。类`_Fwd_list_node<_Tp>`是单链表的节点类，在该类中既有指向下一个节点的指针`next`（通过继承的方式获得），也能存储数据（`storage`用于存储数据）。这样的设计有两个好处：一是方便操作，增加头节点（也称哨兵节点）能够简化操作，尤其是在插入和删除操作中；二是节省不必要的内存，头结点也可以统一为`_Fwd_list_node<_Tp>`，但这样的话会头节点就存储了没有必要的数据（`storage`）。

**问：从类图上看，`forward_list`没有一处使用到`_Fwd_list_node<_Tp>`，难道链表中不存储数据吗？**

答案是否定的，如下图所示：

![fwd-fig2](/fwd-fig2.png)

实际上，除了`head_`指向的头节点之外，所有的节点都是有数据的，即除了头节点之外，所有的节点的类型都是`_Fwd_list_node<_Tp>`。值得注意的是，所有的指针`next`的类型都是`_Fwd_list_node_base`。这无所谓，因为c++允许父类类型的指针指向子类，例如下面的代码是合理的：

```c++
class NodeBase {
public:
    NodeBase* next;
};

class Node: public NodeBase {
public:
    int data;
    Node() = default;
    explicit Node(int d): NodeBase(), data(d) {}
};

int main() {
    NodeBase* p = new NodeBase;
    p->next = new Node(1);              // 父类指针指向子类
    (p->next)->data;                    // 报错
    static_cast<Node*>(p->next)->data;  // 正确
    return 0;
}
```