//
// Created by DELL on 2024/7/30.
//

#include <cstddef>
#include <cstring>
#include <algorithm>

#ifndef CPPBABYSTL_BABY_DEQUE_H
#define CPPBABYSTL_BABY_DEQUE_H

#ifndef DEQUE_BUF_SIZE
#define DEQUE_BUF_SIZE 512
#endif

static constexpr inline std::size_t
deque_buf_size(std::size_t size) {
    return (size < DEQUE_BUF_SIZE)
           ? std::size_t(DEQUE_BUF_SIZE / size)
           : std::size_t(1);
}

template<typename Tp>
class BDeque {
public:
    using SizeType = std::size_t;
    struct Iterator;

private:
    Tp **map_;             // 中控器数组
    SizeType map_size_{};  // 中控器数组的大小
    Iterator start_;       // 迭代器，指向容器的第一个元素
    Iterator finish_;      // 迭代器，指向容器的最后一个元素的写一个位置

    enum {
        kInitMapSize = 8  // map_数组的初始大小
    };


// @{  // 各类构造函数 / 析构函数
private:
    /*
     * @brief 创建一个map（中控器）数组
     * @param map_size map数组的大小
     * */
    Tp ** create_map_(SizeType map_size) {
        return new Tp *[map_size]{};
    }

    // 创建一个缓冲区
    Tp * create_buff_() {
        return new Tp[deque_buf_size(sizeof(Tp))]{};
    }

    /*
     * @brief 初始化含有 n_elems 个元素的容器
     * @param n_elems 容器元素个数
     * */
    void initialize_map_(SizeType n_elems);

    /*
     * @brief 通过指向一个范围的迭代器[beg, end)构造容器
     * @param beg 迭代器，指向构造来源的起始位置
     * @param end 迭代器，指向构造来源结束位置的下一个位置
     *
     * 该函数只是对元素进行构造，调用前需要确保容器的容量足够容纳
     * end - beg 个元素
     * */
    template<typename InputIter>
    void construct_range_(InputIter beg, InputIter end);

    // 销毁当前容器
    void destroy_() {
        for (SizeType i = 0; i < map_size_; i++) {
            if (map_[i] == nullptr) continue;
            delete[] map_[i];
            map_[i] = nullptr;
        }
        delete[] map_;
        std::memset(this, 0, sizeof(*this));
    }

public:

    // 默认构造函数，创建一个空BDeque容器
    BDeque() : map_(nullptr), map_size_(0) {
        initialize_map_(0);
    }

    // 拷贝构造函数
    BDeque(const BDeque &other) : map_(nullptr), map_size_(0) {
        initialize_map_(other.size());
        construct_range_(other.begin(), other.end());
    }

    // 移动构造函数
    BDeque(BDeque &&other) noexcept
            : map_(other.map_),
              map_size_(other.map_size_),
              start_(other.start_),
              finish_(other.finish_) {
        other.initialize_map_(0);
    }

    // 创建一个大小为 cnt 的 BDeque 容器，值全为 Tp 类型默认值
    explicit BDeque(SizeType cnt) : map_(nullptr), map_size_(0) {
        initialize_map_(cnt);
    }

    // 创建一个大小为 cnt 的 BDeque 容器，其值全为val
    BDeque(SizeType cnt, const Tp &val) : map_(nullptr), map_size_(0) {
        initialize_map_(cnt);
        for (Iterator it = start_; it != finish_; ++it) {
            new(it.cur) Tp(val);  // 原位构造
        }
    }

    // 通过初始化列表构造容器
    BDeque(std::initializer_list<Tp> ilist) {
        initialize_map_(ilist.size());
        construct_range_(ilist.begin(), ilist.end());
    }

    // 析构函数
    ~BDeque() { destroy_(); }
// @}  // 各类构造函数 / 析构函数


// @{  // 与赋值相关的操作
private:
    /*
     * @brief 将某个迭代器范围内的所有元素赋值给当前容器
     * @param beg 迭代器，指向数据源的第一个元素
     * @param end 迭代器，指向数据源最后一个元素的下一个位置
     *
     * 注意：在调用此函数前需要保证当前容器有足够的空间（通过调用reserve_elements_at_back_
     * 或reserve_elements_at_front_以保证有足够的空闲空间）
     * */
    template<typename InputIter>
    void assign_range_(InputIter beg, InputIter end);

public:

    // 拷贝赋值函数
    BDeque & operator=(const BDeque &other) {
        if (this == &other) return *this;

        if (other.size() > this->size()) {
            // other容器的元素 > 当前容器元素，在当前容器中预留足够的空间
            reserve_elements_at_back_(other.size() - this->size());
        }
        assign_range_(other.begin(), other.end());
        return *this;
    }

    // 移动赋值函数
    BDeque & operator=(BDeque &&other) noexcept {
        this->destroy_();  // 销毁当前对象
        std::memcpy(this, &other, sizeof(other));
        std::memset(&other, 0, sizeof(other));
        return *this;
    }

    // 将一个初始化列表赋值给当前容器
    BDeque & operator=(std::initializer_list<Tp> init_list) {
        if (init_list.size() > this->size()) {
            reserve_elements_at_back_(init_list.size() - this->size());
        }
        assign_range_(init_list.begin(), init_list.end());
        return *this;
    }

    /*
     * @brief 将容器的内容替换为长度为cnt，值全为val
     * @param cnt 容器的新大小
     * @param val 用来初始化容器的值
     * */
    void assign(SizeType cnt, const Tp &val);

    // 用一个初始化列表来替换掉容器中的内容
    void assign(std::initializer_list<Tp> init_list) {
        if (this->size() < init_list.size()) {
            reserve_elements_at_back_(init_list.size() - this->size());
        }
        assign_range_(init_list.begin(), init_list.end());
    }
// @}  // 与赋值相关的操作


// @{  // 与元素访问相关的操作
public:

    // 返回指定索引的元素引用，索引越界属于未定义行为
    Tp & operator[](SizeType idx) {
        return *((start_ + idx).cur);
    }

    const Tp & operator[](SizeType idx) const {
        return *((start_ + idx).cur);
    }

     // 访问指定索引的元素，带边界检查
    Tp & at(SizeType idx) {
        if (idx >= this->size())
            throw std::out_of_range("BDeque::at");
        return *((start_ + idx).cur);
    }

    const Tp & at(SizeType idx) const {
        if (idx >= this->size())
            throw std::out_of_range("BDeque::at");
        return *((start_ + idx).cur);
    }

    // 返回容器中首元素的引用
    Tp & front() {
        return this->operator[](0);
    }

    const Tp & front() const {
        return this->operator[](0);
    }

    // 返回容器中最后一个元素的引用，在空容器上调用back()属于未定义的行为
    Tp & back() {
        return this->operator[](this->size() - 1);
    }

    Tp & back() const {
        return this->operator[](this->size() - 1);
    }
// @}  // 与元素访问相关的操作


// @{  // 与容器容量相关的函数
public:

    // 返回容器中的元素数
    SizeType size() const noexcept {
        if (start_.node == nullptr || finish_.node == nullptr)
            return SizeType(0);
        SizeType buff_size = deque_buf_size(sizeof(Tp));
        SizeType total_buff_cnt = (finish_.node - start_.node + 1) * buff_size;
        return total_buff_cnt - (start_.cur - start_.first) - (finish_.last - finish_.cur);
    }

    // 检查容器是否为空
    bool empty() const {
        return start_ == finish_;
    }
// @}  // 与容器容量相关的函数


// @{  // 向容器中添加元素的相关操作
private:
    /*
     * @brief 在容器的尾部预定n_elems个元素的空间
     * @param n_elems 预留的空间大小
     * */
    void reserve_elements_at_back_(SizeType n_elems);

    /*
     * @brief 在容器的头部预留n_elems个元素的空间
     * @param n_elems 预留的空间大小
     * */
    void reserve_elements_at_front_(SizeType n_elems);

    /*
     * @brief 重新分配中控器map的内容
     * @param n_nodes 重新需要创建的节点数目
     * @param at_front n_nodes个节点是否在前面创建
     *
     * 如果 n_nodes > 剩余空节点数  ==> 重新创建一个原来两倍大小的新map
     * 如果 n_nodes <= 剩余空节点数 ==> 只需重新分配map即可
     * */
    void reallocate_map_(SizeType n_nodes, bool at_front);

    /*
     * @brief 重新创建一个新的map，并将旧map的数据搬迁到新map上
     * @param new_map_size 新map的大小
     *
     * 注意：必须保证新map_的大小一定大于旧map_的大小
     * */
    void recreate_map_(SizeType new_map_size);

    /*
     * @brief 将[beg, end)内的所有元素前移step个单位
     * @param beg 迭代器，指向待移动范围首元素位置
     * @param end 迭代器，指向待移动范围末尾元素的下一个位置
     * @param step 向前移动的步数
     *
     * 注意：调用该函数前需要保证容器前面预留了足够的位置
     * */
    void move_front_(Iterator beg, Iterator end, SizeType step) {
        Iterator ptr = beg;
        while (ptr < end) {
            std::memcpy((ptr - step).cur, ptr.cur, sizeof(Tp));
            ++ptr;
        }
    }

    /*
     * @brief 将[beg, end)内的所有元素后移step个单位
     * @param beg 迭代器，指向待移动范围首元素位置
     * @param end 迭代器，指向待移动范围最后一个元素的下一个位置
     * @param step 向后移动的步数
     *
     * 注意：调用该函数前需要保证容器后面预留了足够的位置
     * */
    void move_back_(Iterator beg, Iterator end, SizeType step) {
        Iterator ptr = end - 1;
        while (ptr >= beg) {
            std::memcpy((ptr + step).cur, ptr.cur, sizeof(Tp));
            --ptr;
        }
    }
public:

    // 在容器尾部原位构造新元素
    template<typename... Args>
    void emplace_back(Args &&... args) {
        // 在容器尾部预留一个元素的位置
        reserve_elements_at_back_(1);
        if (finish_.cur == finish_.last) {
            finish_.set_node(finish_.node + 1);
            finish_.cur = finish_.first;
        }
        new(finish_.cur) Tp(std::forward<Args>(args)...);
        ++finish_.cur;
    }

    // 向容器尾部插入一个元素
    void push_back(const Tp &val) {
        this->emplace_back(val);
    }

    void push_back(Tp &&val) {
        this->emplace_back(val);
    }

    // 在容器头部原位构造函数
    template<typename... Args>
    void emplace_front(Args &&... args) {
        // 在容器头部预留一个元素的位置
        reserve_elements_at_front_(1);
        --start_;
        new(start_.cur) Tp(std::forward<Args>(args)...);
    }

    // 向容器头部插入一个元素
    void push_front(const Tp &val) {
        this->emplace_front(val);
    }

    void push_front(Tp &&val) {
        this->emplace_front(val);
    }

    /*
     * @brief 在指定索引处插入元素
     * @param idx 索引，元素的插入位置（在idx前插入）
     * @param val 要插入的元素值
     *
     * 当idx >= size()时，均视为在容器尾部插入
     * */
    void insert(SizeType idx, const Tp &val);

    void insert(SizeType idx, Tp &&val) {
        this->insert(idx, val);
    }

    /*
     * @brief 在指定索引处插入多个元素
     * @param idx 索引，元素的插入位置（在idx前插入）
     * @param cnt 要插入的元素数量
     * @param val 要插入的元素值
     *
     * 当idx >= size()时，均视为在容器尾部插入
     * */
    void insert(SizeType idx, SizeType cnt, const Tp &val);

    // 在容器指定位置插入一整个初始化列表的所有内容
    void insert(SizeType idx, std::initializer_list<Tp> init_list);
// @}  // 向容器中添加元素的相关操作


// @{  // 在容器中删除元素的相关操作
public:

    /*
     * @brief 移除容器的末尾元素
     *
     * 如果移除末尾的元素后导致该元素所在的缓冲区为空，则会
     * 释放该缓冲区的内存；此外，在空容器上调用pop_back()
     * 属于未定义的行为
     * */
    void pop_back();

    /*
     * @brief 移除容器的首元素
     *
     * 如果移除容器的首元素后导致该元素所在的缓冲区为空，则会
     * 释放该缓冲区的内存；此外，在空容器上调用pop_front()
     * 属于未定义的行为
     * */
    void pop_front();

    /*
     * @brief 在容器中删除指定范围的元素
     * @param beg_idx 索引，待删除的第一个元素
     * @param end_idx 索引，待删除的最后一个元素的下一个位置
     *
     * 如果[beg_idx, end_idx)非法，会抛出越界异常（注意，
     * end_idx >= size()是合法的）；如果删除[beg_idx, end_idx)
     * 后，缓冲区为空的节点都将被释放
     * */
    void erase(SizeType beg_idx, SizeType end_idx);

    // 删除容器中指定索引的元素
    void erase(SizeType idx) {
        this->erase(idx, idx + 1);
    }

    // 清除容器中所有的元素
    void clear() {
        this->erase(0, this->size());
    }
// @}  // 在容器中删除元素的相关操作


// @{  // 修改容器的相关操作
public:

    // 重设容器大小以容纳cnt个元素；如果容器的长度大于cnt，将cnt之后
    // 的所有元素全部删除；如果容器的长度小于cnt，向容器中添加元素直到
    // 其长度为cnt，添加的元素值为类型的默认值；如果容器的长度为cnt，
    // 则不执行任何操作
    void resize(SizeType cnt);

    // 重设容器大小以容纳cnt个元素；如果容器的长度大于cnt，将cnt之后
    // 的所有元素全部删除；如果容器的长度小于cnt，向容器中添加元素直到
    // 其长度为cnt，添加的元素值为val；如果容器的长度为cnt，则不执行
    // 任何操作
    void resize(SizeType cnt, const Tp &val);

    /*
     * @brief 互换两容器的内容
     * @param other 与当前容器交换内容的容器
     * */
    void swap(BDeque &other) noexcept {
        if (this == &other) return;
        std::swap(map_, other.map_);
        std::swap(map_size_, other.map_size_);
        std::swap(start_, other.start_);
        std::swap(finish_, other.finish_);
    }
// @}  // 修改容器的相关操作


// @{  // 与迭代器相关的操作
public:

    Iterator begin() const { return Iterator(start_); }
    Iterator begin() { return Iterator(start_); }
    Iterator end() const { return Iterator(finish_); }
    Iterator end() { return Iterator(finish_); }
// @}  // 与迭代器相关的操作
};


// @{  // 与类 BDeque 相关的非成员函数
// 重载 == 运算符
template<typename Tp>
bool operator==(const BDeque<Tp> &dq1, const BDeque<Tp> &dq2) {
    if (dq1.size() != dq2.size()) return false;
    auto iter1 = dq1.begin();
    auto iter2 = dq2.begin();
    while (iter1 != dq1.end() && iter2 != dq2.end()) {
        if (*iter1 != *iter1) return false;
        ++iter1;
        ++iter2;
    }
    return true;
}
// @}  // 与类 BDeque 相关的非成员函数


// @{  // 类 BDeque 迭代器的实现
template<typename Tp>
struct BDeque<Tp>::Iterator {
    Tp *cur, *first, *last, **node;

    Iterator()
            : cur(nullptr),
              first(nullptr),
              last(nullptr),
              node(nullptr) {}

    Iterator(Tp *buf_ptr, Tp **node_ptr)
            : cur(buf_ptr),
              first(*node_ptr),
              last(*node_ptr + deque_buf_size(sizeof(Tp))),
              node(node_ptr) {}

    Iterator(const BDeque::Iterator &iter) noexcept
            : cur(iter.cur),
              first(iter.first),
              last(iter.last),
              node(iter.node) {}

    Tp & operator*() const noexcept {
        return *cur;
    }

    Tp * operator->() const noexcept {
        return cur;
    }

    Iterator & operator++() noexcept {
        ++cur;
        if (cur == last) {
            set_node(node + 1);
            cur = first;
        }
        return *this;
    }

    Iterator operator++(int) noexcept {
        Iterator tmp = *this;
        ++*this;
        return tmp;
    }

    Iterator & operator--() noexcept {
        if (cur == first) {
            set_node(node - 1);
            cur = last;
        }
        --cur;
        return *this;
    }

    Iterator operator--(int) noexcept {
        Iterator tmp = *this;
        --*this;
        return tmp;
    }

    Iterator & operator+=(ptrdiff_t n) noexcept {
        ptrdiff_t offset = cur + n - first;
        ptrdiff_t buff_len = deque_buf_size(sizeof(Tp));
        // (... + buff_len) % buff_len: 防止出现负数
        ptrdiff_t buff_offset = (offset % buff_len + buff_len) % buff_len;
        ptrdiff_t node_offset = offset >= 0 ? offset / buff_len
                                            : -((-offset - 1) / buff_len) - 1;

        set_node(node + node_offset);
        cur = first + buff_offset;
        return *this;
    }

    Iterator operator+(ptrdiff_t n) const noexcept {
        Iterator tmp = *this;
        return (tmp += n);
    }

    Iterator & operator-=(ptrdiff_t n) noexcept {
        return *this += -n;
    }

    Iterator operator-(ptrdiff_t n) const noexcept {
        Iterator tmp = *this;
        return tmp -= n;
    }

    Iterator & operator[](ptrdiff_t n) const noexcept {
        return *(*this + n);
    }

    // 重载==, !=, >, >=, <和<= 6个比较运算符
    // 不同于std::deque（类外重载），我们直接类内重载
    bool operator==(const Iterator &other) const {
        return cur == other.cur;
    }

    bool operator!=(const Iterator &other) const {
        return cur != other.cur;
    }

    bool operator>(const Iterator &other) const {
        if (node == other.node)
            return cur > other.cur;
        return node > other.node;
    }

    bool operator>=(const Iterator &other) const {
        if (node == other.node)
            return cur >= other.cur;
        return node >= other.node;
    }

    bool operator<(const Iterator &other) const {
        if (node == other.node)
            return cur < other.cur;
        return node < other.node;
    }

    bool operator<=(const Iterator &other) const {
        if (node == other.node)
            return cur <= other.cur;
        return node <= other.node;
    }

    void set_node(Tp **new_node) noexcept {
        node = new_node;
        first = *new_node;
        if (first == nullptr) {
            last = nullptr;
        } else {
            last = first + deque_buf_size(sizeof(Tp));
        }
    }
};
// @}  // 类 BDeque 迭代器的实现


// @{  // 在类 BDeque 中声明的部分成员函数的实现
template<typename Tp>
void BDeque<Tp>::resize(BDeque::SizeType cnt, const Tp &val) {
    SizeType len = this->size();
    if (cnt > len) {
        for (SizeType i = 0; i < cnt - len; i++) {
            this->emplace_back(val);
        }
    } else if (cnt < len) {
        this->erase(cnt, len);
    }
}

template<typename Tp>
void BDeque<Tp>::resize(BDeque::SizeType cnt) {
    SizeType len = this->size();
    if (cnt > len) {
        for (SizeType i = 0; i < cnt - len; i++) {
            this->emplace_back();
        }
    } else if (cnt < len) {
        this->erase(cnt, len);
    }
}

template<typename Tp>
void BDeque<Tp>::erase(BDeque::SizeType beg_idx, BDeque::SizeType end_idx) {
    if (beg_idx >= size())
        throw std::out_of_range("beg_idx must less than size()");
    if (beg_idx >= end_idx) return;
    if (end_idx > size()) end_idx = size();

    // 依次析构掉[beg_idx, end_idx)内的所有元素
    for (SizeType idx = beg_idx; idx < end_idx; ++idx) {
        (start_ + idx).cur->~Tp();
    }

    // 移动元素以填补[beg_idx, end_idx)的空白
    if (beg_idx > this->size() - end_idx) {
        // [end_idx, size())前移比[0, beg_idx)后移更高效
        move_front_(start_ + end_idx, finish_, end_idx - beg_idx);

        // 修改迭代器finish_的指向
        Tp **empty_node = finish_.node;
        finish_ -= end_idx - beg_idx;
        if (finish_.cur == finish_.first) {
            finish_.set_node(finish_.node - 1);
            finish_.cur = finish_.last;
        }

        // 释放掉因删除元素导致的空缓冲区节点
        for (Tp **node = empty_node; node > finish_.node; --node) {
            delete[] *node;
            *node = nullptr;
        }
    } else {
        // [0, beg_idx)后移比[end_idx, size())前移更高效
        move_back_(start_, start_ + beg_idx, end_idx - beg_idx);

        // 修改迭代器start_的指向
        Tp **empty_node = start_.node;
        start_ += end_idx - beg_idx;

        // 释放掉因删除元素导致的空缓冲区节点
        for (Tp **node = empty_node; node < start_.node; ++node) {
            delete[] *node;
            *node = nullptr;
        }
    }
}

template<typename Tp>
void BDeque<Tp>::pop_front() {
    start_.cur->~Tp();
    if (start_.cur == start_.last - 1) {
        Tp **tmp = start_.node;
        ++start_;

        delete[] *tmp;
        *tmp = nullptr;
    } else ++start_;
}

template<typename Tp>
void BDeque<Tp>::pop_back() {
    if (finish_.cur == finish_.first) {
        Tp **tmp = finish_.node;
        finish_.set_node(finish_.node - 1);
        finish_.cur = finish_.last - 1;

        delete[] *tmp;
        *tmp = nullptr;
    } else {
        --finish_;
    }
    finish_.cur->~Tp();  // 调用该元素的析构函数
}

template<typename Tp>
void BDeque<Tp>::insert(BDeque::SizeType idx, initializer_list<Tp> init_list) {
    if (idx >= this->size() / 2) {
        // [idx, size())内所有元素后移init_list.size()个单位，并将
        // init_list内所有元素依序插入到容器的索引idx处
        reserve_elements_at_back_(init_list.size());
        if (idx >= this->size()) {
            for (const auto &it: init_list) this->push_back(it);
            return;
        }

        Iterator ptr = start_ + idx;
        move_back_(ptr, finish_, init_list.size());
        for (const auto &it: init_list) {
            new(ptr.cur) Tp(it);
            ++ptr;
        }

        // 修改迭代器finish_的指向
        finish_ += init_list.size();
        if (finish_.cur == finish_.first) {
            finish_.set_node(finish_.node - 1);
            finish_.cur = finish_.last;
        }
    } else {
        // [...idx)内的所有元素前移init_list.size()个单位，并将init_list
        // 内所有元素插入到索引 idx - 1 处
        reserve_elements_at_front_(init_list.size());

        // 插入元素
        Iterator ptr = start_ + idx - init_list.size();
        move_front_(start_, start_ + idx, init_list.size());
        for (const auto &it: init_list) {
            new(ptr.cur) Tp(it);
            ++ptr;
        }

        // 修改迭代器start_的指向
        start_ -= init_list.size();
    }
}

template<typename Tp>
void BDeque<Tp>::insert(BDeque::SizeType idx, BDeque::SizeType cnt, const Tp &val) {
    if (idx >= this->size() / 2) {
        // [idx, size())内所有元素向后移动cnt位，并将val插入在idx处
        reserve_elements_at_back_(cnt);
        if (idx >= this->size()) {
            for (SizeType i = 0; i < cnt; i++)
                this->push_back(val);
            return;
        }

        Iterator ptr = start_ + idx;
        move_back_(ptr, finish_, cnt);
        for (SizeType i = 0; i < cnt; i++) {
            new(ptr.cur) Tp(val);
            ++ptr;
        }

        // 修改迭代器finish_的指向
        finish_ += cnt;
        if (finish_.cur == finish_.first) {
            finish_.set_node(finish_.node - 1);
            finish_.cur = finish_.last;
        }
    } else {
        // [start_, idx)内所有元素前移cnt位，并将val插入到idx - 1前
        reserve_elements_at_front_(cnt);
        move_front_(start_, start_ + idx, cnt);

        Iterator ptr = start_ + idx - 1;
        for (SizeType i = 0; i < cnt; i++) {
            new(ptr.cur) Tp(val);
            --ptr;
        }

        // 修改迭代器start_的指向
        start_ -= cnt;
    }
}

template<typename Tp>
void BDeque<Tp>::insert(BDeque::SizeType idx, const Tp &val) {
    if (idx >= this->size() / 2) {
        // [idx, size())内的所有元素后移一个单位，将val插入到idx处
        reserve_elements_at_back_(1);
        if (idx >= this->size()) {
            this->push_back(val);
            return;
        }

        move_back_(start_ + idx, finish_, 1);
        new((start_ + idx).cur) Tp(val);

        // 修改迭代器finish_的指向
        if (finish_.cur == finish_.last) {
            finish_.set_node(finish_.node + 1);
            finish_.cur = finish_.first;
        }
        ++finish_.cur;
    } else {
        // [start_, idx)内所有元素前移一个单位，并将元素插入到位置idx - 1处
        reserve_elements_at_front_(1);
        move_front_(start_, start_ + idx, 1);
        new((start_ + idx - 1).cur) Tp(val);

        // 修改start_的指向
        --start_;
    }
}

template<typename Tp>
void BDeque<Tp>::assign(BDeque::SizeType cnt, const Tp &val) {
    if (this->size() < cnt) {
        reserve_elements_at_back_(cnt - this->size());
    }

    Iterator ptr = start_;
    for (SizeType i = 0; i < cnt; i++) {
        if (ptr < finish_)
            ptr.cur->~Tp();     // 调用当前对象的析构函数
        new(ptr.cur) Tp(val);   // 原位构造
        ++ptr;
    }

    // 删除掉多余的元素
    if (cnt < this->size())
        this->erase(cnt, this->size());
    else {
        finish_ = ptr;
        if (finish_.cur == finish_.first) {
            finish_.set_node(finish_.node - 1);
            finish_.cur = finish_.last;
        }
    }
}

template<typename Tp>
template<typename InputIter>
void BDeque<Tp>::assign_range_(InputIter beg, InputIter end) {
    Iterator ptr = start_;
    while (beg != end) {
        if (ptr < finish_) {
            ptr.cur->~Tp();
        }
        new(ptr.cur) Tp(*beg);  // 原位构造
        ++ptr;
        ++beg;
    }

    // 析构掉多余的元素
    Iterator tmp = ptr;
    while (ptr < finish_) {
        ptr.cur->~Tp();
        ++ptr;
    }
    finish_ = tmp;
}

template<typename Tp>
void BDeque<Tp>::recreate_map_(BDeque::SizeType new_map_size) {
    Tp **tmp = create_map_(new_map_size);
    // 将原来map中的内容复制到新map中
    std::memcpy(tmp, map_, map_size_ * sizeof(Tp *));

    // 记录原来两个迭代器指向的节点
    SizeType beg_idx = start_.node - map_;
    SizeType end_idx = finish_.node - map_;

    // 释放掉老map_的内存空间
    std::memset(map_, 0, map_size_ * sizeof(Tp *));
    delete[] map_;

    // 更新内容
    map_ = tmp;
    map_size_ = new_map_size;
    start_.cur = *(map_ + beg_idx) + (start_.cur - start_.first);
    start_.set_node(map_ + beg_idx);
    finish_.cur = *(map_ + end_idx) + (finish_.cur - finish_.first);
    finish_.set_node(map_ + end_idx);
}

template<typename Tp>
void BDeque<Tp>::reallocate_map_(BDeque::SizeType n_nodes, bool at_front) {
    SizeType full_nodes = finish_.node - start_.node + 1;
    if (n_nodes + full_nodes + 2 > map_size_) {
        // 需要重新创建一个 map
        SizeType new_map_size = std::max(n_nodes + full_nodes + 2, 2 * map_size_);
        recreate_map_(new_map_size);
    }

    // 重新分配map中的内容
    Tp **beg = map_ + (map_size_ - (full_nodes + n_nodes)) / 2;
    if (at_front) {
        beg += n_nodes;
    }
    Tp **end = beg + full_nodes - 1;
    std::memmove(beg, start_.node, full_nodes * sizeof(Tp *));
    for (Tp **ptr = map_; ptr < beg; ptr++) *ptr = nullptr;
    for (Tp **ptr = end + 1; ptr < map_ + map_size_; ptr++) {
        *ptr = nullptr;
    }


    // 重新设置两个迭代器指向的节点
    start_.cur = *beg + (start_.cur - start_.first);
    start_.set_node(beg);
    finish_.cur = *end + (finish_.cur - finish_.first);
    finish_.set_node(end);
}

template<typename Tp>
void BDeque<Tp>::reserve_elements_at_front_(BDeque::SizeType n_elems) {
    // start_所指向的节点足够容纳n_elems个元素 ==> 无需任何操作
    if (n_elems <= start_.cur - start_.first) return;

    SizeType buff_size = deque_buf_size(sizeof(Tp));
    // 容纳n_elems个元素需要至少重新开辟n_nodes个节点
    SizeType n_nodes = (n_elems - (start_.cur - start_.first)) / buff_size;
    if ((n_elems - (start_.cur - start_.first)) % buff_size != 0) n_nodes++;

    // start_指向的节点前最多还能创建rest_nodes个节点
    SizeType rest_nodes = start_.node - map_;
    if (n_nodes >= rest_nodes) {
        reallocate_map_(n_nodes, true);
    }

    Tp **ptr = start_.node - 1;
    for (SizeType i = 0; i < n_nodes; i++) {
        if (*ptr == nullptr)
            *ptr = create_buff_();  // 创建新节点
        --ptr;
    }
}

template<typename Tp>
void BDeque<Tp>::reserve_elements_at_back_(BDeque::SizeType n_elems) {
    // finish_所指向的节点足够容纳n_elems个元素 ==> 无需任何操作
    if (n_elems <= finish_.last - finish_.cur) return;

    SizeType buff_size = deque_buf_size(sizeof(Tp));
    // 容纳n_elems个元素需要至少重新开辟n_nodes个节点
    SizeType n_nodes = (n_elems - (finish_.last - finish_.cur)) / buff_size;
    if ((n_elems - (finish_.last - finish_.cur)) % buff_size != 0) n_nodes++;

    // finish_指向的节点后最多还能创建rest_nodes个节点
    SizeType rest_nodes = map_size_ - (finish_.node - map_ + 1);
    if (n_nodes >= rest_nodes) {
        // 需要创建的节点数 > finish后剩余的节点数，需要重新调整map
        reallocate_map_(n_nodes, false);
    }

    Tp **ptr = finish_.node + 1;
    for (SizeType i = 0; i < n_nodes; i++) {
        if (*ptr == nullptr)
            *ptr = create_buff_();  // 创建新节点
        ++ptr;
    }
}

template<typename Tp>
template<typename InputIter>
void BDeque<Tp>::construct_range_(InputIter beg, InputIter end) {
    Iterator iter = start_;
    while (beg != end) {
        new(iter.cur) Tp(*beg);  // 原位构造
        ++beg;
        ++iter;
    }
    finish_ = iter;
}

template<typename Tp>
void BDeque<Tp>::initialize_map_(BDeque::SizeType n_elems) {
    SizeType n_nodes = n_elems / deque_buf_size(sizeof(Tp)) + 1;
    map_size_ = std::max(n_nodes + 2, SizeType(kInitMapSize));

    map_ = create_map_(map_size_);  // 创建map

    // 创建缓冲区
    Tp **beg = map_ + (map_size_ - n_nodes) / 2;
    Tp **end = beg;
    for (SizeType i = 0; i < n_nodes; i++) {
        *end = create_buff_();
        ++end;
    }

    // 设置两个迭代器
    start_.set_node(beg);
    start_.cur = *beg;
    finish_.set_node(end - 1);
    finish_.cur = finish_.first + n_elems % deque_buf_size(sizeof(Tp));
}
// @}  // 在类 BDeque 中声明的部分成员函数的实现

#endif //CPPBABYSTL_BABY_DEQUE_H
