## 各类构造函数/析构函数

```c++
BList();                                // 1
explicit BList(SizeType cnt);          // 2
BList(SizeType cnt, const Tp& val);    // 3
BList(const BList& other);              // 4
BList(BList&& other);                   // 5
BList(std::initializer_list<Tp> init);  // 6
~BList();                               // 7
```

| 编号 | 函数功能                                    | 时间复杂度            |
| ---- | ------------------------------------------- | --------------------- |
| 1    | 默认构造函数                                | O(1)                  |
| 2    | 创建长度为`cnt`的容器，值为类型`Tp`的默认值 | O(`cnt`)              |
| 3    | 创建长度为`cnt`的容器，容器的值全为`val`    | O(`cnt`)              |
| 4    | 拷贝构造函数                                | 与`other`的大小成线性 |
| 5    | 移动构造函数                                | O(1)                  |
| 6    | 使用初始化列表创建容器                      | 与`init`的大小成线性  |
| 7    | 析构函数                                    | 与容器的大小成线性    |

**函数参数**

- `cnt`：新创建的容器大小
- `val`：用以初始化容器元素的值
- `other`：用作初始化容器元素来源的另一容器
- `init`：用于初始化容器的初始化列表

**注解**

​	除了默认构造函数和移动构造函数之外，其余构造函数的构造方式都是通过不断地调用`emplace_back`函数向容器的末尾插入元素。需要注意的是，移动构造函数调用完成后，`other`将会被置为空。

**示例**

```c++
#include "src/BList.h"

int main() {
    BList<int> list1;                    // 默认构造
    BList<int> list2(3);                 // 打印输出0,0,0
    BList<int> list3(4, 2);              // 打印输出2,2,2,2
    BList<int> list4(list2);             // 打印输出0,0,0
    BList<int> list5(std::move(list3));  // 打印输出2,2,2,2  list3为空
    BList<int> list6{1, 2, 3, 4};        // 打印输出1,2,3,4

    int idx = 1;
    for (const auto& l : {list1, list2, list3, list4, list5, list6}) {
        cout << "list" << idx++ << ": [";
        for (const auto& it : l) {
            cout << it << " ";
        }
        cout << "], size = " << l.size() << endl;
    }
    return 0;
}
```

## 赋值相关的函数

```c++
BList& operator=(const BList& other);               // 1
BList& operator=(BList&& other) noexcept;           // 2
BList& operator=(std::initializer_list<Tp> ilist);  // 3
void assign(SizeType cnt, const Tp& val);          // 4
void assign(std::initializer_list<Tp> ilist);       // 5
```

| 编号 | 函数功能                               | 时间复杂度                      |
| ---- | -------------------------------------- | ------------------------------- |
| 1    | 重载赋值运算符                         | 与`other`和当前容器的大小成线性 |
| 2    | 重载移动赋值运算符                     | O(1)                            |
| 3    | 将一个初始化列表中的元素赋值给当前容器 | 与`ilist`和当前容器的大小成线性 |
| 4    | 用指定数量的元素值替换掉容器中的内容   | 与`cnt`和当前容器的大小成线性   |
| 5    | 用初始化列表替换掉当前容器的内容       | 与`ilist`和当前容器的大小成线性 |

**函数参数**

- `other`：用做数据源的另一容器
- `ilist`：用作数据源的初始化列表
- `cnt`：替换后容器的大小
- `val`：用以替换容器元素的值

**注解**

编号为1、3和5的函数有着相同的实现逻辑，即遍历数据源，依次将新元素赋值给容器，因此可以定义一个私有成员函数实现上述逻辑：

```c++
template<typename InputIter>
void assign_range_(InputIter beg, InputIter end);
```

有了`assign_range_`后，以下函数只需要简单的调用即可：

```c++
BList& operator=(const BList& other) {
    if (this == &other) return *this;  // 防止自赋值
    assign_range_(other.begin(), other.end());
    return *this;
}

BList& operator=(std::initializer_list<Tp> ilist) {
    assign_range_(ilist.begin(), ilist.end());
    return *this;
}

void assign(std::initializer_list<Tp> ilist) {
    assign_range_(ilist.begin(), ilist.end());
}
```

**示例**

```c++
#include "src/BList.h"

int main() {
    BList<int> list{1, 2, 3};

    cout << "copy assign:" << endl;
    BList<int> list1{5, 6};
    list = list1;
    cout << "list: ";
    for (const auto& it : list) {
        cout << it << " ";
    }
    cout << ", size = " << list.size() << endl << endl;

    cout << "move assign:" << endl;
    list = BList<int>{7, 8, 9, 0};
    cout << "list: ";
    for (const auto& it : list) {
        cout << it << " ";
    }
    cout << ", size = " << list.size() << endl << endl;

    cout << "initializer list assign:" << endl;
    list = {-1};
    cout << "list: ";
    for (const auto& it : list) {
        cout << it << " ";
    }
    cout << ", size = " << list.size();
    return 0;
}
```

## 元素访问相关操作

```c++
Tp& front();              // 1
const Tp& front() const;  // 2
Tp& back();               // 3
const Tp& back() const;   // 4
```

| 编号 | 函数功能                                                     | 时间复杂度 |
| ---- | ------------------------------------------------------------ | ---------- |
| 1    | 返回容器首元素的引用                                         | O(1)       |
| 2    | 返回容器首元素的引用（当容器对象被const修饰时被调用）        | O(1)       |
| 3    | 返回容器中最后一个元素的引用                                 | O(1)       |
| 4    | 返回容器中最后一个元素的引用（当容器对象被const修饰时被调用） | O(1)       |

**函数参数**

（无）

**注解**

上述4个操作的实现较为简单，详细代码可参考（TODO）

需要注意的是，在空容器上调用front()和back()属于为定义的行为。

**示例**

```c++
#include "src/BList.h"

int main() {
    BList<int> list{1, 2, 3};

    cout << "the first element of list: " << list.front() << endl;
    cout << "the last element of list: " << list.back() << endl;
    return 0;
}
```

## 与容器容量相关操作

```c++
SizeType size() const;  // 1
bool empty() const;      // 2
```

| 编号 | 函数功能         | 时间复杂度 |
| ---- | ---------------- | ---------- |
| 1    | 返回容器的元素数 | O(1)       |
| 2    | 检查容器是否为空 | O(1)       |

**函数参数**

（无）

**注解**

上述两个函数的具体实现代码如下：

```c++
SizeType size() const {
    return size_;
}

bool empty() const {
    return size_ == 0;
}
```

**示例**

```c++
#include "src/BList.h"

int main() {
    BList<int> list{1, 2, 3};
    list.size();   // 返回3
    list.empty();  // 返回false
    return 0;
}
```

## 添加元素的相关操作

```c++
template<typename ...Args> 
void emplace_back(Args... args);                                    // 1

template<typename ...Args> 
void emplace_front(Args... args);                                   // 2

void push_back(const Tp& val);                                      // 3
void push_back(Tp&& val);                                           // 4
void push_front(const Tp& val);                                     // 5
void push_front(Tp&& val);                                          // 6
void insert_(BList::Iterator pos, const Tp& val);                    // 7
void insert_(BList::Iterator pos, Tp&& val);                         // 8
void insert_(BList::Iterator pos, SizeType cnt, const Tp& val);     // 9
void insert_(BList::Iterator pos, std::initializer_list<Tp> ilist);  // 10
```

| 编号 | 函数功能                                       | 时间复杂度            |
| ---- | ---------------------------------------------- | --------------------- |
| 1    | 在容器尾部原位构造新元素                       | O(1)                  |
| 2    | 在容器头部原位构造新元素                       | O(1)                  |
| 3    | 向容器末尾添加新元素                           | O(1)                  |
| 4    | 向容器末尾添加新元素（以移动的方式添加）       | O(1)                  |
| 5    | 向容器头部添加新元素                           | O(1)                  |
| 6    | 向容器头部添加新元素（以移动的方式添加）       | O(1)                  |
| 7    | 在容器的指定位置插入新元素                     | O(1)                  |
| 8    | 在容器的指定位置插入新元素（以移动的方式添加） | O(1)                  |
| 9    | 在容器的指定位置插入多个值为`val`的新元素      | O(`cnt`)              |
| 10   | 在容器的指定位置插入一个初始化列表的所有元素   | 与`ilist`的长度成线性 |

**函数参数**

- `args`：转发到元素构造函数的实参
- `val`：向容器中添加的新元素的值
- `pos`：指向新元素插入位置的迭代器（新元素将插入到`pos`前）
- `cnt`：将要插入元素的数目
- `ilist`：存放所有新插入元素的初始化列表

**注解**

在实现上，上述10个用于向`BList`容器添加新元素成员函数都通过调用该容器的一个私有成员函数完成向容器中添加新元素的功能：

```c++
template<typename ...Args>
void insert_(NodeBase* pos, Args... args);
```

`insert_`函数会在`pos`前创建一个新节点，该节点的对象通过`args`参数完成构造，`insert_`的具体实现参见TODO。

**示例**

```c++
#include "src/BList.h"

int main() {
    BList<int> list = {0, 0, 1};
    list.emplace_back(2);    // list={0,0,1,2}
    list.emplace_front(-1);  // list={-1,0,0,1,2}
    list.push_back(3);       // list={-1,0,0,1,2,3}
    list.push_front(-2);     // list={-2,-1,0,0,1,2,3}

    list.insert_(++list.begin(), 9);       // list={-2,9,-1,0,0,1,2,3}
    list.insert_(list.end(), {100, 100});  // list={-2,9,-1,0,0,1,2,3,100,100}
    return 0;
}
```

## 删除元素的相关操作

```c++
void clear();                               // 1
void erase(Iterator pos);                   // 2
void erase(Iterator beg, Iterator end);     // 3
void pop_front();                           // 4
void pop_back();                            // 5
void resize(SizeType cnt);                 // 6
void resize(SizeType cnt, const Tp& val);  // 7
```

| 编号 | 函数功能                      | 时间复杂度                     |
| ---- | ----------------------------- | ------------------------------ |
| 1    | 清除容器中的所有元素          | 与容器的大小成线性             |
| 2    | 删除指定位置的元素            | O(1)                           |
| 3    | 删除指定范围的所有元素        | 与`beg`和`end`之间的距离成线性 |
| 4    | 移除首元素                    | O(1)                           |
| 5    | 移除末尾元素                  | O(1)                           |
| 6    | 重设容器以容纳`cnt`个元素大小 | O(`size()` + `cnt`)            |
| 7    | 重设容器以容纳`cnt`个元素大小 | O(`size()` + `cnt`)            |

> - 在`pop_back()`和`pop_front()`操作中，与`std::list`有所不同。在`std::list`中，当容器为空时，调用`pop_back()`和`pop_front()`属于未定义的行为；而在`BList`中，当容器为空时`pop_back()`和`pop_front()`不会执行任何操作。
> - 编号为6和7的`resize`函数的区别是：当`cnt`的值大于容器的大小时，编号为6的`resize`函数会添加值为模版类型`Tp`默认值的新元素，而编号为7的`resize`函数则添加值为`val`的新元素。

**函数参数**

- `pos`：指向待删除元素位置的迭代器
- `beg, end`：迭代器，指向某个待删除范围的头和尾，即`[beg, end)`。
- `cnt`：新容器的大小
- `val`：新元素的值

**注解**

编号1-5的所有对容器元素的删除操作均通过调用私有函数`destroy_range_`实现，`destroy_range_`函数的声明如下：

```c++
void destroy_range_(NodeBase* beg, NodeBase* end);
```

调用该函数会销毁掉`[beg, end)`内的所有节点，并调用该节点存储的数据对象的析构函数。

**示例**

```c++
#include "src/BList.h"

int main() {
    BList<int> list = {1, 2, 3, 4, 5, 6, 7};
    list.erase(list.begin());                      // list={2,3,4,5,6,7}
    list.erase(list.begin(), ++(++list.begin()));  // list={4,5,6,7}
    list.pop_front();                              // list={5,6,7}
    list.pop_back();                               // list={5,6}
    return 0;
}
```

## 交换两个链表中的内容`swap`

```c++
void swap(BList& other) noexcept;
```

将当前容器中的内容与另一容器`other`进行交换，该操作不会导致任何的元素发生内容上的拷贝，同时也不会造成迭代器的失效。类似于移动操作，时间和空间复杂度为O(1)。

**示例**

```c++
#include "src/BList.h"

int main() {
    BList<int> list1 = {7, 4, 1};
    BList<int> list2 = {5, 3, 2};
    
    // 操作完成后：
    //  list1={5,3,2}
    //  list2={7,4,1}
    list1.swap(list2);
    return 0;
}
```

## 合并两个有序容器`merge`

```c++
template<class Compare>
void merge(BList&& other, Compare cmp);

template<class Compare>
void merge(BList& other, Compare cmp);

void merge(BList& other);
void merge(BList&& other);
```



## 迭代器

​	在`std::list`中，总共定义了`_List_iterator`和`_List_const_iterator`两个迭代器类。和迭代器名所描述的一样，`_List_iterator`允许修改迭代器指向的元素，而`_List_const_iterator`不允许修改迭代器指向的元素。这样做的目的是为了遵循c++中的常量正确性（const-correctness）原则，以提供类型安全的迭代器。

​	本项目中`BList`容器提供的是一种简化版本的迭代器类`Iterator`，仅仅是为了使得容器支持**基于范围的for循环（Range-based for loop）**，即我们可以使用如下语句遍历容器：

```c++
for (declaration : expression) {
    statement
}
```

`BList`容器的`Iterator`类的声明如下：

```c++
class Iterator {
    private:
            Node* ptr;
    public:
            explicit Iterator(Node* p);
    		explicit Iterator(const NodeBase* p);
			
    		// 重载*运算符
            const Tp& operator*();
            Tp& operator*();
			
    		// 重载前缀++运算符
            const Iterator& operator++() const;
            Iterator& operator++();

            Iterator operator++(int);  // 后缀++运算符
			
    		// 重载前缀--运算符
            const Iterator& operator--() const;
            Iterator& operator--();

            Iterator operator--(int);  // 后缀--运算符
			
    		// 重载==运算符
            bool operator==(const Iterator& other) const;
    
    		// 重载!=运算符
            bool operator!=(const Iterator& other) const;
    };
```

> **注意：**`Iterator`类直接声明在`BList`容器内部，因此可以直接使用模版类型`Tp`，同时`Node<Tp>`也可以简写为`Node`。

​	有了迭代器类`Iterator`后，只需要在容器`BList`内部定义`begin()`和`end()`函数，就可以使`BList`容器支持基于范围的for循环：

```c++
Iterator begin() {
    return Iterator(static_cast<Node*>(head_.next));
}
Iterator begin() const {
    return Iterator(head_.next);
}

Iterator end() {
    // 在循环链表中，头结点即尾节点
    return Iterator(static_cast<Node*>(&head_));
}
Iterator end() const {
    // 在循环链表中，头结点即尾节点
    return Iterator(&head_);
}
```

