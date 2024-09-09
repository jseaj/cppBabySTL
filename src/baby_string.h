//
// Created by DELL on 2024/6/30.
//

#include <stdexcept>
#include <initializer_list>
#include <iostream>
#include <cstring>

#ifndef CPPBABYSTL_BABY_STRING_H
#define CPPBABYSTL_BABY_STRING_H


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

private:

    /*
     * @brief 开辟 new_capacity 大小的空间
     * @param new_capacity 需要开辟的空间大小（实际为 new_capacity + 1）
     * @param old_capacity 原来的空间大小
     *
     * 注意：虽然指定了需要开辟的空间大小为 new_capacity, 但实际分配的
     * 空间不一定是 new_capacity。实际分配的大小将被更新并存储回
     * new_capacity 参数中
     * */
    char *creat_(SizeType &new_capacity) const;

    /*
     * @brief 重新分配大小为 new_capacity 的空间
     *
     * 调用该函数会执行下面3个步骤：
     *   1. 调用 creat_ 创建新的存储空间
     *   2. 将旧空间上的值拷贝到新空间并销释放掉老空间
     *   3. 修改 BString 对象上的相关值
     * */
    void reallocation_(SizeType new_capacity) {
        // 重新分配内存
        auto tmp = creat_(new_capacity);

        // 将原来的字符串拷贝到新内存中
        strcpy(tmp, ptr_);
        destroy_();  // 释放原来的内存
        ptr_ = tmp;
        allocated_capacity_ = new_capacity;
    }


// @{ //  各类构造函数 / 析构函数
private:
    /*
     * @brief 通过字符串s（c风格字符串）构造对象，仅供调用
     * @param s 目标的头指针
     * @param n 字符串的长度
     * */
    void construct_(const char *s, SizeType n);

    // 销毁 ptr_ 指向的堆空间
    void destroy_() {
        if (ptr_ == local_buf_) return;
        delete[] ptr_;
        ptr_ = local_buf_;
        str_len_ = 0;
    }

public:

    // 默认构造函数：创建一个空字符串
    BString() : ptr_(local_buf_), str_len_(0) {
        ptr_[0] = '\0';
    }

    // 拷贝构造函数
    BString(const BString &b_str) : ptr_(local_buf_) {
        construct_(b_str.ptr_, b_str.str_len_);
    }

    // 移动构造函数
    BString(BString &&b_str) noexcept;

    // 通过 std::string 对象进行构造
    explicit BString(const std::string &str)
            : ptr_(local_buf_) {
        construct_(str.c_str(), str.size());
    }

    // 通过 c 风格字符串进行构造
    explicit BString(const char *s, SizeType n)
            : ptr_(local_buf_) {
        SizeType len = strlen(s);
        if (n == npos || n > len) {
            n = len;
        }
        construct_(s, n);
    }

    // 通过 c 风格字符串进行构造
    BString(const char *s)
            : ptr_(local_buf_) {
        construct_(s, strlen(s));
    }

    // 通过 std::string 进行移动构造
    explicit BString(std::string &&str) : ptr_(local_buf_) {
        // 无法修改_str中的内容，因此和 BString(const std::string& str)
        // 操作一样
        construct_(str.c_str(), str.size());
    }

    /*
     * @brief 使用单个字符进行构造
     * @param n 字符的数目
     * @param c 使用的单个字符
     * */
    BString(SizeType n, char c) : ptr_(local_buf_) {
        if (n > kLocalCapacity) {
            ptr_ = new char[n + 1]{};
            allocated_capacity_ = n;
        }

        // 复制字符
        for (SizeType i = 0; i < n; i++) {
            ptr_[i] = c;
        }
        ptr_[n] = '\0';
        str_len_ = n;
    }

    /*
     * @brief 通过初始化列表进行构造
     * @param init_list 初始化列表
     * */
    BString(std::initializer_list<char> init_list) : ptr_(local_buf_) {
        construct_(init_list.begin(), init_list.size());
    }

    // 析构函数
    ~BString() { destroy_(); }
// @}  // 各类构造函数 / 析构函数


// @{  // 与赋值相关的操作
private:

    /*
     * @brief 将 s 串中前 n 个字符赋值给当前 BString（重载=操作符时调用）
     * @param s c风格的字符串
     * @param n s串中需要赋值的长度
     * */
    void assign_(const char *s, SizeType n);

public:

    // 拷贝赋值函数
    BString & operator=(const BString &b_str) {
        if (this == &b_str) return *this;  // 防止自赋值

        assign_(b_str.ptr_, b_str.str_len_);
        return *this;
    }

    // 移动赋值函数
    BString & operator=(BString &&b_str) noexcept;

    // 通过 std::string 对象进行拷贝赋值
    BString & operator=(const std::string &str) {
        assign_(str.c_str(), str.size());
        return *this;
    }

    // 通过 std::string 对象进行移动赋值
    BString & operator=(std::string &&str) {
        // 无法修改 std::string 对象内容，与拷贝赋值相同操作
        assign_(str.c_str(), str.size());
        return *this;
    }

    // 通过c风格字符串进行拷贝赋值
    BString & operator=(const char *s) {
        assign_(s, strlen(s));
        return *this;
    }
// @}  // 与赋值相关的操作


// @{  // 与迭代器相关的操作
public:
    // 直接使用 `数组首地址` 作为迭代器即可

    char* begin() { return ptr_; }
    char* begin() const {return ptr_; }
    char* end() { return ptr_ + this->size(); }
    char* end() const { return ptr_ + this->size(); }
// @}  //  与迭代器相关的操作


// @{  // 与容器容量相关的操作
public:

    // 返回BString中字符的数目
    SizeType size() const { return str_len_; }

    // @brief 返回BString的存储空间
    SizeType capacity() const {
        return ptr_ == local_buf_ ?
               kLocalCapacity : allocated_capacity_;
    }

    // 判断字符串是否为空
    bool empty() const {
        return this->size() == 0;
    }
// @}  // 与容器容量相关的操作


// @{  // 与数据访问相关的操作
public:

    // 重载[]操作符，不同于 std::string，该函数会进行越界检查
    const char & operator[](SizeType idx) const {
        if (idx >= this->size()) {
            throw std::out_of_range("BString::operator[]: Index out of range");
        }
        return ptr_[idx];
    }

    // 重载[]操作符
    char & operator[](SizeType idx) {
        if (idx >= str_len_) {
            throw std::out_of_range("BString::operator[]: Index out of range");
        }
        return ptr_[idx];
    }

    // 返回字符串第一个字符的引用
    const char & front() const noexcept {
        return operator[](0);
    }

    // 返回字符串第一个字符的引用
    char & front() noexcept {
        return operator[](0);
    }

    // 返回字符串第最后一个字符的引用
    const char & back() const noexcept {
        return operator[](this->size() - 1);
    }

    // 返回字符串第最后一个字符
    char & back() noexcept {
        return operator[](this->size() - 1);
    }
// @}  // 与数据访问相关的操作


// @{  // 向容器中增加元素相关的操作
private:

    /*
     * @brief 将 s 串的前 n 个字符追加到当前 BString 后
     * @param s c风格字符串
     * @param n 追加的字符串长度
     * */
    void append_(const char *s, SizeType n);

public:

    // 向当前字符串末尾追加一个 BString 类型的字符串
    BString & append(const BString &b_str) {
        append_(b_str.ptr_, b_str.size());
        return *this;
    }

    // 向当前字符串末尾追加一个 std::string 类型的字符串
    BString & append(const std::string &str) {
        append_(str.c_str(), str.size());
        return *this;
    }

    // 向当前字符串末尾追加一个 c风格 的字符串
    BString & append(const char *s) {
        append_(s, strlen(s));
        return *this;
    }

    // 向当前BString末尾追加一个字符
    BString & push_back(char c) {
        append_(&c, 1);
        return *this;
    }

    // 重载 += 操作符
    BString & operator+=(const BString &b_str) {
        return this->append(b_str);
    }

    // 重载 += 操作符
    BString & operator+=(const std::string &str) {
        return this->append(str);
    }

    // 重载 += 操作符
    BString & operator+=(const char *s) {
        return this->append(s);
    }

    // 重载+=操作符
    BString & operator+=(char c) {
        return this->push_back(c);
    }
// @}  // 向容器中增加元素相关的操作


// @{  // 在容器中删除元素的相关操作
private:
    /*
     * @brief 将 idx 后的 n 个字符删除
     * @param idx 第一个删除点的索引
     * @param n 需要删除的字符数
     *
     * 等价于将当前 BString 中[idx + n, size())前移 n 个单位
     * */
    void erase_(SizeType idx, SizeType n);

public:
    // 删除当前BString的最后一个字符
    BString & pop_back() noexcept {
        if (this->empty()) return *this;

        erase_(this->size() - 1, 1);
        return *this;
    }

    /*
     * @brief 删除当前BString对象从 idx 后的 n 个字符
     * @param idx 要删除的第一个字符的索引
     * @param n 要删除的字符数目
     * @return 当前对象的引用
     * @throw 当 idx 越界时，抛出std::out_of_range异常
     * */
    BString & erase(SizeType idx = 0, SizeType n = npos) {
        if (idx >= this->size()) {
            throw std::out_of_range("BString::erase");
        }
        if (n == npos) {
            // idx之后的字符 ==> 全部删除
            str_len_ = idx;
            ptr_[idx] = '\0';
        } else if (n != 0) {
            n = std::min(this->size() - idx, n);
            erase_(idx, n);
        }
        return *this;
    }

    // 在字符串中删除索引为 idx 的单个字符
    BString & erase(SizeType idx) {
        if (idx >= this->size()) {
            throw std::out_of_range("BString::erase");
        }
        erase_(idx, SizeType(1));
        return *this;
    }
// @}  // 在容器中删除元素的相关操作


// @{  // 与子串和元素查找相关的操作
private:
    /*
     * @brief 在[beg, end)上查找模式串 s 第一次出现的索引
     * @param beg
     * @param end
     * @param s 待匹配的模式串
     * @param n 模式串 s 的长度
     *
     * 采用KMP算法实现，时间复杂度O(m + n), 空间复杂度O(m + n), 参考
     * 链接：https://oi-wiki.org/string/kmp/, 标准库采用暴力算法实
     * 现，时间复杂度为O(m * n), 空间复杂度为O(1)
     * */
    SizeType find_(SizeType beg, SizeType end, const char *s, SizeType n) const;

public:

    /*
     * @brief 在[idx, size())中查找子串 b_str 首次出现的位置
     * @param b_str 待匹配的模式串
     * @param idx 从主串的 idx 后开始查找
     * */
    SizeType find(const BString &b_str, SizeType idx = 0) const {
        if (idx >= this->size()) return npos;
        return find_(idx, this->size(), b_str.ptr_, b_str.size());
    }

    /*
     * @brief 在[idx, size())中查找子串 str 首次出现的位置
     * @param str 待匹配的模式串
     * @param idx 从主串的 idx 后开始查找
     * */
    SizeType find(const std::string &str, SizeType idx = 0) const {
        if (idx >= this->size()) return npos;
        return find_(idx, this->size(), str.c_str(), str.size());
    }

    /*
     * @brief 在[idx, size())中查找子串 s 首次出现的位置
     * @param s 待匹配的模式串
     * @param idx 从主串的 idx 后开始查找
     * */
    SizeType find(const char *s, SizeType idx = 0) const {
        if (idx >= this->size()) return npos;
        return find_(idx, this->size(), s, strlen(s));
    }

    /*
     * @brief 获取一个从 idx 位置开始，长度为 n 的子串
     * @param idx 开始索引
     * @param n 子串的长度
     * @return 新的字符串对象
     * */
    BString substr(SizeType idx, SizeType n = npos) const {
        if (idx >= this->size()) {
            throw std::out_of_range("BString::substr");
        }
        if (n == npos) n = this->size();
        n = std::min(n, this->size() - idx);
        return BString(ptr_ + idx, n);
    }
// @}  //  与子串和元素查找相关的操作


// @{  // 重载各类操作符
public:

    // 重载 << 运算符
    friend std::ostream &
    operator<<(std::ostream &os, const BString &b_str) {
        os << b_str.ptr_;
        return os;
    }

    // 重载 == 操作符
    bool operator==(const BString &b_str) {
        return strcmp(ptr_, b_str.ptr_) == 0;
    }

    //  重载 > 操作符
    bool operator>(const BString &b_str) {
        return strcmp(ptr_, b_str.ptr_) > 0;
    }

    // 重载 >= 操作符
    bool operator>=(const BString &b_str) {
        return strcmp(ptr_, b_str.ptr_) >= 0;
    }

    // 重载 < 操作符
    bool operator<(const BString &b_str) {
        return strcmp(ptr_, b_str.ptr_) < 0;
    }

    // 重载 <= 操作符
    bool operator<=(const BString &b_str) {
        return strcmp(ptr_, b_str.ptr_) <= 0;
    }

    /*
     * @brief 重载+操作符 ==> 拼接两个BString
     * @param b_str1 第一个字符串
     * @param b_str2 第二个字符串
     * @return 拼接后的新字符串
     * */
    friend BString
    operator+(const BString &b_str1, const BString &b_str2) {
        BString str(b_str1);
        str.append(b_str2);
        return str;
    }

    /*
     * @brief 重载+操作符 ==> 拼接BString和string
     * @param b_str1 第一个字符串
     * @param str2 第二个字符串
     * @return 拼接后的新字符串
     * */
    friend BString
    operator+(const BString &b_str1, const std::string &str2) {
        BString str(b_str1);
        str.append(str2);
        return str;
    }

    /*
     * @brief 重载+操作符 ==> 拼接string和BString
     * @param str1 第一个字符串
     * @param b_str2 第二个字符串
     * @return 拼接后的新字符串
     * */
    friend BString
    operator+(const std::string &str1, const BString &b_str2) {
        BString str(str1);
        str.append(b_str2);
        return str;
    }

    /*
     * @brief 重载+操作符 ==> 拼接BString和c风格字符串
     * @param b_str1 第一个字符串
     * @param s2 第二个字符串
     * @return 拼接后的新字符串
     * */
    friend BString
    operator+(const BString &b_str1, const char *s2) {
        BString str(b_str1);
        str.append(s2);
        return str;
    }

    /*
     * @brief 重载+操作符 ==> 拼接c风格字符串和BString
     * @param s1 第一个字符串
     * @param b_str2 第二个字符串
     * @return 拼接后的新字符串
     * */
    friend BString
    operator+(const char *s1, const BString &b_str2) {
        BString str(s1);
        str.append(b_str2);
        return str;
    }
// @}  // 重载各类操作符
};

#endif //CPPBABYSTL_BABY_STRING_H



/* BSting 对象部分成员函数的实现 */
void BString::construct_(const char *s, BString::SizeType n) {
    if (n > kLocalCapacity) {
        // 在堆上分配空间
        ptr_ = new char[n + 1]{};  // 多一位存储'\0'
        allocated_capacity_ = n;
    }

    // 复制字符
    strncpy(ptr_, s, n);
    str_len_ = n;
}

void BString::assign_(const char *s, BString::SizeType n) {
    SizeType capacity = this->capacity();

    if (n > capacity) {
        // 需要重新分配内存
        reallocation_(n);
    }

    // 复制字符串
    strncpy(ptr_, s, n);
    ptr_[n] = '\0';  // strncpy不保证复制'\0'
    str_len_ = n;
}

void BString::erase_(BString::SizeType idx, BString::SizeType n) {
    if (idx >= this->size()) return;

    if (idx + n >= this->size()) {
        ptr_[idx] = '\0';
        str_len_ = idx;
    } else {
        SizeType cnt = this->size() - (idx + n);
        memmove(ptr_ + idx, ptr_ + idx + n, cnt);
        ptr_[this->size() - n] = '\0';
        str_len_ = this->size() - n;
    }
}

BString::SizeType BString::find_(
        BString::SizeType beg, BString::SizeType end, const char *s, BString::SizeType n) const {
    if (n == 0) return beg;  // 空串能够匹配上任意字符串
    if (n > end - beg) return npos;

    // 计算前缀函数
    const auto prefix_func = [](const char *str, SizeType n) -> SizeType * {
        // n为字符串str的长度
        auto pi = new SizeType[n];
        pi[0] = SizeType(0);
        for (SizeType i = 1; i < n; i++) {
            SizeType j = pi[i - 1];
            while (j > 0 && str[i] != str[j]) j = pi[j - 1];
            if (str[i] == str[j]) j++;
            pi[i] = j;
        }
        return pi;
    };

    SizeType m = end - beg;
    auto cur = new char[m + 1 + n];
    strncpy(cur, s, n);
    cur[n] = '\0';  // 分隔符，既不能在 ptr_ 中也不能在 s_ 中存在
    strncpy(cur + n + 1, ptr_ + beg, m);

    SizeType *lps = prefix_func(cur, m + 1 + n);
    for (SizeType i = n + 1; i <= m + n; i++) {
        if (lps[i] == n) return i - 2 * n + beg;
    }

    // 释放临时变量的内存
    delete[] cur;
    delete[] lps;

    return npos;
}

char *BString::creat_(BString::SizeType &new_capacity) const {
    SizeType old_capacity = this->capacity();
    // TODO: new_capacity > max_size() ==> 抛出异常
    if (new_capacity > old_capacity && new_capacity < 2 * old_capacity) {
        new_capacity = 2 * old_capacity;
        // 标准实现中还需要判断是否比容量上限max_size()大
        // 如果大则只开辟max_size()的大小
    }
    return new char[new_capacity + 1];
}

void BString::append_(const char *s, BString::SizeType n) {
    SizeType len = this->size() + n;
    SizeType capacity = this->capacity();

    if (len > capacity) {
        // 重新分配内存
        reallocation_(len);
    }

    // 将_s串前_n个字符拷贝到_M_ptr后
    strncpy(ptr_ + str_len_, s, n);
    ptr_[len] = '\0';
    str_len_ = len;
}

BString::BString(BString &&b_str) noexcept {
    ptr_ = local_buf_;
    SizeType len = b_str.size();
    if (len <= kLocalCapacity) {
        // 字符串存储在栈上，直接复制
        strcpy(ptr_, b_str.ptr_);
    } else {
        ptr_ = b_str.ptr_;
        allocated_capacity_ = b_str.allocated_capacity_;

        // 将b_str置为空
        b_str.ptr_ = b_str.local_buf_;
        b_str.str_len_ = 0;
        b_str.ptr_[0] = '\0';
    }
    str_len_ = len;
}

BString &BString::operator=(BString &&b_str) noexcept {
    if (this == &b_str) return *this;

    SizeType len = b_str.size();
    if (b_str.ptr_ == b_str.local_buf_) {
        strcpy(ptr_, b_str.ptr_);
    } else {
        ptr_ = b_str.ptr_;
        allocated_capacity_ = b_str.allocated_capacity_;

        b_str.ptr_ = b_str.local_buf_;
        b_str.str_len_ = 0;
    }
    str_len_ = len;
    return *this;
}
