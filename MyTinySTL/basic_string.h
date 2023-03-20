#ifndef MYTINYSTL_BASIC_STRING_H_
#define MYTINYSTL_BASIC_STRING_H_

// 这个头文件包含一个模板类 basic_string
// 用于表示字符串类型

#include <iostream>

#include "iterator.h"
#include "memory.h"
#include "functional.h"
#include "exceptdef.h"

namespace mystl
{

// char_traits

template <class CharType>
struct char_traits
{//这是一个空间分配器，对其内部的空间有以下几种操作，注意以下的操作都是在内存上直接操作的，不需要对象的参与
    //相反，对象的某些方法需要借助以下操作来实现
  typedef CharType char_type;//这里的char_type是指char、wchar、char16、char32
  
  static size_t length(const char_type* str)
  {//string底层是字符数组，以字符0结尾，所以计算string长度的时候是计算字符个数
    size_t len = 0;
    for (; *str != char_type(0); ++str)
      ++len;
    return len;
  }

  static int compare(const char_type* s1, const char_type* s2, size_t n)
  {
    for (; n != 0; --n, ++s1, ++s2)
    {
      if (*s1 < *s2)
        return -1;
      if (*s2 < *s1)//这里不用大于的原因是有些类重载了小于号而没有重载大于号
        return 1;
    }
    return 0;
  }

  static char_type* copy(char_type* dst, const char_type* src, size_t n)
  {
    MYSTL_DEBUG(src + n <= dst || dst + n <= src);
    //这里调用了assert宏定义，如果它里面的条件返回错误，代码会终止运行，并且会把源文件，错误的代码，以及行号，都输出来。
    //正确的话就继续运行
    //这里是说两块地址不能重叠
    char_type* r = dst;
    for (; n != 0; --n, ++dst, ++src)
      *dst = *src;
    return r;
  }

  static char_type* move(char_type* dst, const char_type* src, size_t n)
  {//转移n个字符
    char_type* r = dst;
    //注意这里对于空间重叠的处理
    if (dst < src)
    {//目标地址小于源地址，那么可以直接复制过去
      for (; n != 0; --n, ++dst, ++src)
        *dst = *src;
    }
    else if (src < dst)
    {//目标地址大于源地址，就应该从后往前复制，避免把未复制的字符给覆盖了
      dst += n;
      src += n;
      for (; n != 0; --n)
        *--dst = *--src;
    }
    return r;
  }

  static char_type* fill(char_type* dst, char_type ch, size_t count)
  {
    char_type* r = dst;
    for (; count > 0; --count, ++dst)
      *dst = ch;
    return r;
  }
};

// Partialized. char_traits<char>
template <> 
struct char_traits<char>//针对char的特例化版本，比上一个类模板快
{
  typedef char char_type;

  static size_t length(const char_type* str) noexcept
  { return std::strlen(str); }

  static int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
  { return std::memcmp(s1, s2, n); }

  static char_type* copy(char_type* dst, const char_type* src, size_t n) noexcept
  {
    MYSTL_DEBUG(src + n <= dst || dst + n <= src);
    return static_cast<char_type*>(std::memcpy(dst, src, n));
  }

  static char_type* move(char_type* dst, const char_type* src, size_t n) noexcept
  {
    return static_cast<char_type*>(std::memmove(dst, src, n));
  }

  static char_type* fill(char_type* dst, char_type ch, size_t count) noexcept
  { 
    return static_cast<char_type*>(std::memset(dst, ch, count));
  }
};

// Partialized. char_traits<wchar_t>
template <>
struct char_traits<wchar_t>
{
  typedef wchar_t char_type;

  static size_t length(const char_type* str) noexcept
  {
    return std::wcslen(str);
  }

  static int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
  {
    return std::wmemcmp(s1, s2, n);
  }

  static char_type* copy(char_type* dst, const char_type* src, size_t n) noexcept
  {
    MYSTL_DEBUG(src + n <= dst || dst + n <= src);
    return static_cast<char_type*>(std::wmemcpy(dst, src, n));
  }

  static char_type* move(char_type* dst, const char_type* src, size_t n) noexcept
  {
    return static_cast<char_type*>(std::wmemmove(dst, src, n));
  }

  static char_type* fill(char_type* dst, char_type ch, size_t count) noexcept
  { 
    return static_cast<char_type*>(std::wmemset(dst, ch, count));
  }
};

// Partialized. char_traits<char16_t>
template <>
struct char_traits<char16_t>
{
  typedef char16_t char_type;

  static size_t length(const char_type* str) noexcept
  {
    size_t len = 0;
    for (; *str != char_type(0); ++str)
      ++len;
    return len;
  }

  static int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
  {
    for (; n != 0; --n, ++s1, ++s2)
    {
      if (*s1 < *s2)
        return -1;
      if (*s2 < *s1)
        return 1;
    }
    return 0;
  }

  static char_type* copy(char_type* dst, const char_type* src, size_t n) noexcept
  {
    MYSTL_DEBUG(src + n <= dst || dst + n <= src);
    char_type* r = dst;
    for (; n != 0; --n, ++dst, ++src)
      *dst = *src;
    return r;
  }

  static char_type* move(char_type* dst, const char_type* src, size_t n) noexcept
  {
    char_type* r = dst;
    if (dst < src)
    {
      for (; n != 0; --n, ++dst, ++src)
        *dst = *src;
    }
    else if (src < dst)
    {
      dst += n;
      src += n;
      for (; n != 0; --n)
        *--dst = *--src;
    }
    return r;
  }

  static char_type* fill(char_type* dst, char_type ch, size_t count) noexcept
  {
    char_type* r = dst;
    for (; count > 0; --count, ++dst)
      *dst = ch;
    return r;
  }
};

// Partialized. char_traits<char32_t>
template <>
struct char_traits<char32_t>
{
  typedef char32_t char_type;

  static size_t length(const char_type* str) noexcept
  {
    size_t len = 0;
    for (; *str != char_type(0); ++str)
      ++len;
    return len;
  }

  static int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
  {
    for (; n != 0; --n, ++s1, ++s2)
    {
      if (*s1 < *s2)
        return -1;
      if (*s2 < *s1)
        return 1;
    }
    return 0;
  }

  static char_type* copy(char_type* dst, const char_type* src, size_t n) noexcept
  {
    MYSTL_DEBUG(src + n <= dst || dst + n <= src);
    char_type* r = dst;
    for (; n != 0; --n, ++dst, ++src)
      *dst = *src;
    return r;
  }

  static char_type* move(char_type* dst, const char_type* src, size_t n) noexcept
  {
    char_type* r = dst;
    if (dst < src)
    {
      for (; n != 0; --n, ++dst, ++src)
        *dst = *src;
    }
    else if (src < dst)
    {
      dst += n;
      src += n;
      for (; n != 0; --n)
        *--dst = *--src;
    }
    return r;
  }

  static char_type* fill(char_type* dst, char_type ch, size_t count) noexcept
  {
    char_type* r = dst;
    for (; count > 0; --count, ++dst)
      *dst = ch;
    return r;
  }
};

// 初始化 basic_string 尝试分配的最小 buffer 大小，可能被忽略
#define STRING_INIT_SIZE 32

// 模板类 basic_string
// 参数一代表字符类型，参数二代表萃取字符类型的方式，缺省使用 mystl::char_traits
template <class CharType, class CharTraits = mystl::char_traits<CharType>>
class basic_string
{
public:
  typedef CharTraits                               traits_type;
  typedef CharTraits                               char_traits;

  typedef mystl::allocator<CharType>               allocator_type;//对于不同的对象，内存分配器的类型也不相同
  typedef mystl::allocator<CharType>               data_allocator;

  typedef typename allocator_type::value_type      value_type;//实际上就是CharType
  typedef typename allocator_type::pointer         pointer;
  typedef typename allocator_type::const_pointer   const_pointer;
  typedef typename allocator_type::reference       reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type       size_type;
  typedef typename allocator_type::difference_type difference_type;

  typedef value_type*                              iterator;//把底层指针命名为迭代器
  typedef const value_type*                        const_iterator;
  typedef mystl::reverse_iterator<iterator>        reverse_iterator;
  typedef mystl::reverse_iterator<const_iterator>  const_reverse_iterator;

  allocator_type get_allocator() { return allocator_type(); }//allocator_type是实例化的类，加括号以后就是构造一个对象
  //这里用的是合成的构造函数
  //assert是运行时断言，只有在执行到assert时才会进行判断。而static_assert是在编译时进行断言。所以断言的条件必须是编译时即可确定
  static_assert(std::is_pod<CharType>::value, "Character type of basic_string must be a POD");
  //对于同一个string，其底层字符类型必须和萃取字符方法所获得的字符类型相同
  static_assert(std::is_same<CharType, typename traits_type::char_type>::value,
                "CharType must be same as traits_type::char_type");

public:
  // 末尾位置的值，例:
  // if (str.find('a') != string::npos) { /* do something */ }
  static constexpr size_type npos = static_cast<size_type>(-1);

private://注意这里是私有的，对象无法访问
  iterator  buffer_;  // 储存字符串的起始位置，其实是个指针
  size_type size_;    // 大小
  size_type cap_;     // 容量

public:
  // 构造、复制、移动、析构函数

  basic_string() noexcept
  { try_init(); }

  basic_string(size_type n, value_type ch)
    :buffer_(nullptr), size_(0), cap_(0)
  {
    fill_init(n, ch);
  }

  basic_string(const basic_string& other, size_type pos)//复制构造
    :buffer_(nullptr), size_(0), cap_(0)
  {
    init_from(other.buffer_, pos, other.size_ - pos);
  }
  basic_string(const basic_string& other, size_type pos, size_type count)
    :buffer_(nullptr), size_(0), cap_(0)
  {
    init_from(other.buffer_, pos, count);
  }

  basic_string(const_pointer str)
    :buffer_(nullptr), size_(0), cap_(0)
  {//init_from需要指定开始位置和字符个数，这里是完全复制
    init_from(str, 0, char_traits::length(str));
  }
  basic_string(const_pointer str, size_type count)
    :buffer_(nullptr), size_(0), cap_(0)
  {
    init_from(str, 0, count);
  }

  template <class Iter, typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
  //注意这是一个构造函数，没有返回值，这是用两个迭代器之间的值去初始化basic_string
  //Iter是模板构造函数的模板参数，另一个参数是一个类型，如果Iter迭代器是输入迭代器，那么类型为int，并且有默认值0
  //否则的话另一个参数不存在
  basic_string(Iter first, Iter last)
  { copy_init(first, last, iterator_category(first)); }

  basic_string(const basic_string& rhs) 
    :buffer_(nullptr), size_(0), cap_(0)
  {
    init_from(rhs.buffer_, 0, rhs.size_);
  }
  basic_string(basic_string&& rhs) noexcept
    :buffer_(rhs.buffer_), size_(rhs.size_), cap_(rhs.cap_)
  {//转移构造函数，临时对象会被销毁
    rhs.buffer_ = nullptr;
    rhs.size_ = 0;
    rhs.cap_ = 0;
  }

  basic_string& operator=(const basic_string& rhs);//拷贝赋值
  basic_string& operator=(basic_string&& rhs) noexcept;//移动赋值

  basic_string& operator=(const_pointer str);
  basic_string& operator=(value_type ch);

  ~basic_string() { destroy_buffer(); }//析构函数非虚

public:
  // 迭代器相关操作
  iterator/*这是返回值*/               begin()         noexcept
  { return buffer_; }
  const_iterator         begin()   const noexcept
  { return buffer_; }//对于常对象来说，this指针是const的，无法调用上面那个普通的成员函数，只能调用这个常成员函数
                     //而常对象和非常对象都能调用这个const函数，那为什么还需要上面那个函数？
  iterator               end()           noexcept
  { return buffer_ + size_; }//最后一个字符后面的位置
  const_iterator         end()     const noexcept
  { return buffer_ + size_; }

  reverse_iterator       rbegin()        noexcept
  //end()返回尾指针，reverse_iterator是mystl::reverse_iterator<iterator>这个类的别名，end()返回的指针作为这个类构造函数的参数
      //从而构造出一个reverse_iterator的对象，返回
  { return reverse_iterator(end()); }
  const_reverse_iterator rbegin()  const noexcept
  { return const_reverse_iterator(end()); }
  reverse_iterator       rend()          noexcept
  { return reverse_iterator(begin()); }
  const_reverse_iterator rend()    const noexcept
  { return const_reverse_iterator(begin()); }

  const_iterator         cbegin()  const noexcept
  { return begin(); }
  const_iterator         cend()    const noexcept
  { return end(); }
  const_reverse_iterator crbegin() const noexcept
  { return rbegin(); }
  const_reverse_iterator crend()   const noexcept
  { return rend(); }

  // 容量相关操作
  bool      empty()    const noexcept
  { return size_ == 0; }

  size_type size()     const noexcept
  { return size_; }
  size_type length()   const noexcept
  { return size_; }
  size_type capacity() const noexcept
  { return cap_; }
  size_type max_size() const noexcept
  { return static_cast<size_type>(-1); }//-1的二进制表示全都是1，将其转换为size_type就能得到最大的值，这样在各种机器上都能适用

  void      reserve(size_type n);
  //reserve：调整string大小，使之可以容纳n个元素，如果当前容量小于n，则扩展容量至n，其他情况则不进行存储重新分配，对容量没有影响
  void      shrink_to_fit();
  //减少容器的容量以适应其大小并销毁超出容量的所有元素。


  // 访问元素相关操作
  reference       operator[](size_type n) //注意这里返回的是引用，所以才能改变元素值，参数是下标
  {
    MYSTL_DEBUG(n <= size_);//下标不能大于size
    if (n == size_)
      *(buffer_ + n) = value_type();//value_type是一个类型，这里在内存末尾的位置构造了一个value_type对象，
                                    //因为内存分配器的底层对象和value_type是一样的,所以可以构造
    return *(buffer_ + n); 
  }
  const_reference operator[](size_type n) const
  { 
    MYSTL_DEBUG(n <= size_);
    if (n == size_)
      *(buffer_ + n) = value_type();
    return *(buffer_ + n);
  }

  reference       at(size_type n) 
  { 
    THROW_OUT_OF_RANGE_IF(n >= size_, "basic_string<Char, Traits>::at()"
                          "subscript out of range");
    return (*this)[n]; 
  }
  const_reference at(size_type n) const 
  {
    THROW_OUT_OF_RANGE_IF(n >= size_, "basic_string<Char, Traits>::at()"
                          "subscript out of range");
    return (*this)[n]; 
  }

  reference       front() 
  { 
    MYSTL_DEBUG(!empty());//字符替换，执行点仍然在这里相当于assert(!empty())
    return *begin(); 
  }
  const_reference front() const 
  { 
    MYSTL_DEBUG(!empty());
    return *begin(); 
  }

  reference       back() 
  {
    MYSTL_DEBUG(!empty()); 
    return *(end() - 1); //实际上是对指针减1
  }
  const_reference back()  const
  {
    MYSTL_DEBUG(!empty()); 
    return *(end() - 1);
  }

  const_pointer   data()  const noexcept
  { return to_raw_pointer(); }//返回原始的头指针
  const_pointer   c_str() const noexcept
  { return to_raw_pointer(); }//c_str()就是将string转化为字符串数组,生成一个const指针

  // 添加删除相关操作

  // insert
  iterator insert(const_iterator pos, value_type ch);
  iterator insert(const_iterator pos, size_type count, value_type ch);

  template <class Iter>
  iterator insert(const_iterator pos, Iter first, Iter last);


  // push_back / pop_back
  void     push_back(value_type ch)
  { append(1, ch); }
  void     pop_back()
  {
    MYSTL_DEBUG(!empty());//非空才能弹出元素
    --size_;//这里并没有把内存空间析构掉，而是直接减少了size，即使那个内存上已经存了对象也忽略掉了，最后会一起析构
  }

  // append
  basic_string& append(size_type count, value_type ch);//加入count个ch字符

  basic_string& append(const basic_string& str)//把另一个string加进来
  { return append(str, 0, str.size_); }//调用513行的append
  basic_string& append(const basic_string& str, size_type pos)//把另一个string[pos:end)的部分加进来
  { return append(str, pos, str.size_ - pos); }//调用513行的append
  basic_string& append(const basic_string& str, size_type pos, size_type count);

  basic_string& append(const_pointer s)//s是字符串数组的首地址
  { return append(s, char_traits::length(s)); }//length函数的输入参数是首地址，计算字符串数组的长度，调用的是下面的append
  basic_string& append(const_pointer s, size_type count);

  template <class Iter, typename std::enable_if<
    mystl::is_input_iterator<Iter>::value, int>::type = 0>//Iter需要是输入迭代器，由于前向、双向、随机访问迭代器都是继承自输入迭代器，因此这些迭代器都可以
  basic_string& append(Iter first, Iter last)
  { return append_range(first, last); }

  // erase /clear
  iterator erase(const_iterator pos);//返回迭代器，删除从pos开始的所有字符
  iterator erase(const_iterator first, const_iterator last);

  // resize
  void resize(size_type count)
  { resize(count, value_type()); }//默认构造一个value_type对象？
  void resize(size_type count, value_type ch);

  void     clear() noexcept
  { size_ = 0; }//只是把size变成0了

  // basic_string 相关操作

  // compare
  int compare(const basic_string& other) const;
  int compare(size_type pos1, size_type count1, const basic_string& other) const;
  int compare(size_type pos1, size_type count1, const basic_string& other,
              size_type pos2, size_type count2 = npos) const;
  int compare(const_pointer s) const;
  int compare(size_type pos1, size_type count1, const_pointer s) const;
  int compare(size_type pos1, size_type count1, const_pointer s, size_type count2) const;

  // substr
  basic_string substr(size_type index, size_type count = npos)//因为构造的是一个临时对象，所以不能返回引用或指针
  {
    count = mystl::min(count, size_ - index);//最多到末尾
    return basic_string(buffer_ + index, buffer_ + index + count);//构造一个临时对象返回
  }

  // replace
  basic_string& replace(size_type pos, size_type count, const basic_string& str)
  {
    THROW_OUT_OF_RANGE_IF(pos > size_, "basic_string<Char, Traits>::replace's pos out of range");
    return replace_cstr(buffer_ + pos, count, str.buffer_, str.size_);//调用1763行的函数
  }
  basic_string& replace(const_iterator first, const_iterator last, const basic_string& str)
  {
    MYSTL_DEBUG(begin() <= first && last <= end() && first <= last);//同类型指针可以比大小
    return replace_cstr(first, static_cast<size_type>(last - first), str.buffer_, str.size_);
  }

  basic_string& replace(size_type pos, size_type count, const_pointer str)//用一个basic_string对象或字符串数组首地址都能代表这个内存中的字符串
  {
    THROW_OUT_OF_RANGE_IF(pos > size_, "basic_string<Char, Traits>::replace's pos out of range");
    return replace_cstr(buffer_ + pos, count, str, char_traits::length(str));
  }
  basic_string& replace(const_iterator first, const_iterator last, const_pointer str)
  {
    MYSTL_DEBUG(begin() <= first && last <= end() && first <= last);
    return replace_cstr(first, static_cast<size_type>(last - first), str, char_traits::length(str));
  }

  basic_string& replace(size_type pos, size_type count, const_pointer str, size_type count2)
  {
    THROW_OUT_OF_RANGE_IF(pos > size_, "basic_string<Char, Traits>::replace's pos out of range");
    return replace_cstr(buffer_ + pos, count, str, count2);
  }
  basic_string& replace(const_iterator first, const_iterator last, const_pointer str, size_type count)
  {
    MYSTL_DEBUG(begin() <= first && last <= end() && first <= last);
    return replace_cstr(first, static_cast<size_type>(last - first), str, count);

  }

  basic_string& replace(size_type pos, size_type count, size_type count2, value_type ch)
  {
    THROW_OUT_OF_RANGE_IF(pos > size_, "basic_string<Char, Traits>::replace's pos out of range");
    return replace_fill(buffer_ + pos, count, count2, ch);//填充
  }
  basic_string& replace(const_iterator first, const_iterator last, size_type count, value_type ch)
  {
    MYSTL_DEBUG(begin() <= first && last <= end() && first <= last);
    return replace_fill(first, static_cast<size_type>(last - first), count, ch);
  }

  basic_string& replace(size_type pos1, size_type count1, const basic_string& str,
                        size_type pos2, size_type count2 = npos)
  {
    THROW_OUT_OF_RANGE_IF(pos1 > size_ || pos2 > str.size_,
                          "basic_string<Char, Traits>::replace's pos out of range");
    return replace_cstr(buffer_ + pos1, count1, str.buffer_ + pos2, count2);
  }

  template <class Iter, typename std::enable_if<
    mystl::is_input_iterator<Iter>::value, int>::type = 0>
  basic_string& replace(const_iterator first, const_iterator last, Iter first2, Iter last2)
  {
    MYSTL_DEBUG(begin() <= first && last <= end() && first <= last);
    return replace_copy(first, last, first2, last2);
  }

  // reverse
  void reverse() noexcept;

  // swap
  void swap(basic_string& rhs) noexcept;

  // 查找相关操作

  // find
  size_type find(value_type ch, size_type pos = 0)                             const noexcept;
  size_type find(const_pointer str, size_type pos = 0)                         const noexcept;
  size_type find(const_pointer str, size_type pos, size_type count)            const noexcept;
  size_type find(const basic_string& str, size_type pos = 0)                   const noexcept;

  // rfind
  size_type rfind(value_type ch, size_type pos = npos)                         const noexcept;
  size_type rfind(const_pointer str, size_type pos = npos)                     const noexcept;
  size_type rfind(const_pointer str, size_type pos, size_type count)           const noexcept;
  size_type rfind(const basic_string& str, size_type pos = npos)               const noexcept;

  // find_first_of
  size_type find_first_of(value_type ch, size_type pos = 0)                    const noexcept;
  size_type find_first_of(const_pointer s, size_type pos = 0)                  const noexcept;
  size_type find_first_of(const_pointer s, size_type pos, size_type count)     const noexcept;
  size_type find_first_of(const basic_string& str, size_type pos = 0)          const noexcept;

  // find_first_not_of
  size_type find_first_not_of(value_type ch, size_type pos = 0)                const noexcept;
  size_type find_first_not_of(const_pointer s, size_type pos = 0)              const noexcept;
  size_type find_first_not_of(const_pointer s, size_type pos, size_type count) const noexcept;
  size_type find_first_not_of(const basic_string& str, size_type pos = 0)      const noexcept;

  // find_last_of
  size_type find_last_of(value_type ch, size_type pos = 0)                     const noexcept;
  size_type find_last_of(const_pointer s, size_type pos = 0)                   const noexcept;
  size_type find_last_of(const_pointer s, size_type pos, size_type count)      const noexcept;
  size_type find_last_of(const basic_string& str, size_type pos = 0)           const noexcept;

  // find_last_not_of
  size_type find_last_not_of(value_type ch, size_type pos = 0)                 const noexcept;
  size_type find_last_not_of(const_pointer s, size_type pos = 0)               const noexcept;
  size_type find_last_not_of(const_pointer s, size_type pos, size_type count)  const noexcept;
  size_type find_last_not_of(const basic_string& str, size_type pos = 0)       const noexcept;

  // count
  size_type count(value_type ch, size_type pos = 0) const noexcept;

public:
  // 重载 operator+= 
  basic_string& operator+=(const basic_string& str)
  { return append(str); }//调用append拼接两个string
  basic_string& operator+=(value_type ch)
  { return append(1, ch); }//调用append拼接一个字符
  basic_string& operator+=(const_pointer str)
  { return append(str, str + char_traits::length(str)); }

  // 重载 operator >> / operatror <<

  friend std::istream& operator >> (std::istream& is, basic_string& str)//友元函数，相当于cin>>str，注意这个函数不是模板类的成员
  {
    value_type* buf = new value_type[4096];//4096个字符的数组
    is >> buf;//输入进去
    basic_string tmp(buf);//利用字符数组构造string
    str = std::move(tmp);//转移资源到目的string
    delete[]buf;//注意这里的析构，否则会有内存泄漏，tmp是临时变量，离开函数之后就被析构了
    return is;//这是为了方便连续输入，cin>>str1>>str2
  }

  friend std::ostream& operator << (std::ostream& os, const basic_string& str)
  {
    for (size_type i = 0; i < str.size_; ++i)
      os << *(str.buffer_ + i);//逐个字符输出
    return os;//为了连续输出
  }

private:
  // helper functions

  // init / destroy 
  void          try_init() noexcept;

  void          fill_init(size_type n, value_type ch);

  template <class Iter>
  void          copy_init(Iter first, Iter last, mystl::input_iterator_tag);
  template <class Iter>
  void          copy_init(Iter first, Iter last, mystl::forward_iterator_tag);

  void          init_from(const_pointer src, size_type pos, size_type n);

  void          destroy_buffer();

  // get raw pointer
  const_pointer to_raw_pointer() const;

  // shrink_to_fit
  void          reinsert(size_type size);

  // append
  template <class Iter>
  basic_string& append_range(Iter first, Iter last);

  // compare
  int compare_cstr(const_pointer s1, size_type n1, const_pointer s2, size_type n2) const;

  // replace
  basic_string& replace_cstr(const_iterator first, size_type count1, const_pointer str, size_type count2);
  basic_string& replace_fill(const_iterator first, size_type count1, size_type count2, value_type ch);
  template <class Iter>
  basic_string& replace_copy(const_iterator first, const_iterator last, Iter first2, Iter last2);

  // reallocate
  void          reallocate(size_type need);
  iterator      reallocate_and_fill(iterator pos, size_type n, value_type ch);
  iterator      reallocate_and_copy(iterator pos, const_iterator first, const_iterator last);
};

/*****************************************************************************************/

// 复制赋值操作符
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>&//返回值
basic_string<CharType, CharTraits>:://表明是哪个类的成员函数
operator=(const basic_string& rhs)
{
  if (this != &rhs)//避免自赋值
  {
    basic_string tmp(rhs);//调用构造函数
    swap(tmp);//调用move函数转移资源，相当于先把this给转移出去，再把rhs转移给this，再把转移出去的给rhs，
              //这里就要求不能自赋值，因为this转移出去后对象就不存在了，rhs不存在
  }
  return *this;
}

// 移动赋值操作符
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>&
basic_string<CharType, CharTraits>::
operator=(basic_string&& rhs) noexcept
{
  destroy_buffer();//先销毁this，这里不需要考虑自赋值，因为销毁是绝对安全的
  buffer_ = rhs.buffer_;//然后赋值
  size_ = rhs.size_;
  cap_ = rhs.cap_;
  rhs.buffer_ = nullptr;//在销毁rhs
  rhs.size_ = 0;
  rhs.cap_ = 0;
  return *this;
}

// 用一个字符串赋值
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>&
basic_string<CharType, CharTraits>::
operator=(const_pointer str)
{
  const size_type len = char_traits::length(str);
  if (cap_ < len)
  {
    auto new_buffer = data_allocator::allocate(len + 1);//新申请一块内存
    data_allocator::deallocate(buffer_);//销毁当前内存
    buffer_ = new_buffer;//变成新的内存地址
    cap_ = len + 1;
  }
  char_traits::copy(buffer_, str, len);//内存拷贝
  size_ = len;
  return *this;
}

// 用一个字符赋值
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>&
basic_string<CharType, CharTraits>::
operator=(value_type ch)
{//和上面的一样
  if (cap_ < 1)
  {
    auto new_buffer = data_allocator::allocate(2);
    data_allocator::deallocate(buffer_);
    buffer_ = new_buffer;
    cap_ = 2;
  }
  *buffer_ = ch;
  size_ = 1;
  return *this;
}

// 预留储存空间
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>:://返回值为空
reserve(size_type n)
{
  if (cap_ < n)
  {
    THROW_LENGTH_ERROR_IF(n > max_size(), "n can not larger than max_size()"
                          "in basic_string<Char,Traits>::reserve(n)");
    auto new_buffer = data_allocator::allocate(n);//申请一块新内存
    char_traits::move(new_buffer, buffer_, size_);//把内容转移过去，move是内存操作，更快
    buffer_ = new_buffer;//新地址
    cap_ = n;
  }
}

// 减少不用的空间
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
shrink_to_fit()
{
  if (size_ != cap_)
  {
    reinsert(size_);
  }
}

// 在 pos 处插入一个元素
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::iterator//返回一个迭代器
basic_string<CharType, CharTraits>::
insert(const_iterator pos, value_type ch)
{
  iterator r = const_cast<iterator>(pos);//强行去掉const
  if (size_ == cap_)
  {//已经装满了，要重新申请内存并填充
    return reallocate_and_fill(r, 1, ch);
  }
  char_traits::move(r + 1, r, end() - r);//否则的话，把pos后面的原有字符都往后移动一个位置
  ++size_;
  *r = ch;//然后把这个位置插入ch
  return r;//返回插入首地址
}

// 在 pos 处插入 n 个元素
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::iterator
basic_string<CharType, CharTraits>::
insert(const_iterator pos, size_type count, value_type ch)
{
  iterator r = const_cast<iterator>(pos);
  if (count == 0)
    return r;
  if (cap_ - size_ < count)
  {//内存不够
    return reallocate_and_fill(r, count, ch);
  }
  if (pos == end())
  {//直接在末尾填充
    char_traits::fill(end(), ch, count);
    size_ += count;
    return r;
  }
  char_traits::move(r + count, r, count);//移动count个位置
  char_traits::fill(r, ch, count);//填充
  size_ += count;
  return r;
}

// 在 pos 处插入 [first, last) 内的元素
template <class CharType, class CharTraits>
template <class Iter>//模板函数本身的模板参数
typename basic_string<CharType, CharTraits>::iterator
basic_string<CharType, CharTraits>::
insert(const_iterator pos, Iter first, Iter last)
{
  iterator r = const_cast<iterator>(pos);
  const size_type len = mystl::distance(first, last);
  if (len == 0)
    return r;
  if (cap_ - size_ < len)
  {
    return reallocate_and_copy(r, first, last);
  }
  if (pos == end())
  {
    mystl::uninitialized_copy(first, last, end());//Iter保证是输入迭代器？
    size_ += len;
    return r;
  }
  char_traits::move(r + len, r, len);
  mystl::uninitialized_copy(first, last, r);
  size_ += len;
  return r;
}

// 在末尾添加 count 个 ch
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>& 
basic_string<CharType, CharTraits>::
append(size_type count, value_type ch)
{
  THROW_LENGTH_ERROR_IF(size_ > max_size() - count,//这里为什么不是和cap比较？意思是申请所有内存都不够才会报错？
                        "basic_string<Char, Tratis>'s size too big");
  if (cap_ - size_ < count)
  {
    reallocate(count);//重新申请内存
  }
  char_traits::fill(buffer_ + size_, ch, count);//在末尾添加
  size_ += count;
  return *this;
}

// 在末尾添加 [str[pos] str[pos+count]) 一段
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>& 
basic_string<CharType, CharTraits>::
append(const basic_string& str, size_type pos, size_type count)
{
  THROW_LENGTH_ERROR_IF(size_ > max_size() - count,
                        "basic_string<Char, Tratis>'s size too big");
  if (count == 0)
    return *this;
  if (cap_ - size_ < count)
  {
    reallocate(count);
  }
  char_traits::copy(buffer_ + size_, str.buffer_ + pos, count);
  size_ += count;
  return *this;
}

// 在末尾添加 [s, s+count) 一段
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>& 
basic_string<CharType, CharTraits>::
append(const_pointer s, size_type count)
{
  THROW_LENGTH_ERROR_IF(size_ > max_size() - count,
                        "basic_string<Char, Tratis>'s size too big");
  if (cap_ - size_ < count)
  {
    reallocate(count);
  }
  char_traits::copy(buffer_ + size_, s, count);//直接拷贝内存
  size_ += count;
  return *this;
}

// 删除 pos 处的元素
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::iterator
basic_string<CharType, CharTraits>::
erase(const_iterator pos)
{
  MYSTL_DEBUG(pos != end());//pos不能等于end，因为这里没有元素
  iterator r = const_cast<iterator>(pos);
  char_traits::move(r, pos + 1, end() - pos - 1);//把后面的元素往前面覆盖，而不是全部移动
  --size_;
  return r;//返回删除后原位置的迭代器
}

// 删除 [first, last) 的元素
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::iterator
basic_string<CharType, CharTraits>::
erase(const_iterator first, const_iterator last)
{
  if (first == begin() && last == end())
  {
    clear();
    return end();
  }
  const size_type n = end() - last;
  iterator r = const_cast<iterator>(first);//为啥要先去掉const？因为move、copy这些函数会改变迭代器的值，所以不能是const
  char_traits::move(r, last, n);//char_traits是别名，已经声明过模板的参数，所以char_traits直接代表一个类，而move是这个类中的静态成员函数
  //所以这里相当于是mystl::char_traits<CharType>::move(r,last,n)，会根据不同CharType分别调用特例化的函数
  //直接把last后面的元素移动到first的位置上实现覆盖
  size_ -= (last - first);
  return r;
}

// 重置容器大小
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
resize(size_type count, value_type ch)
{
  if (count < size_)
  {//直接删除多余的
      this->erase(buffer_ + count, buffer_ + size_);
  }
  else
  {//再加上一些字符
      this->append(count - size_, ch);
  }
}

// 比较两个 basic_string，小于返回 -1，大于返回 1，等于返回 0
template <class CharType, class CharTraits>
int basic_string<CharType, CharTraits>::
compare(const basic_string& other) const
{//其实就是比较字符数组
  return compare_cstr(buffer_, size_, other.buffer_, other.size_);
}

// 从 pos1 下标开始的 count1 个字符跟另一个 basic_string 比较
template <class CharType, class CharTraits>
int basic_string<CharType, CharTraits>::
compare(size_type pos1, size_type count1, const basic_string& other) const
{
  auto n1 = mystl::min(count1, size_ - pos1);//最多比较这么多字符
  return compare_cstr(buffer_ + pos1, n1, other.buffer_, other.size_);
}

// 从 pos1 下标开始的 count1 个字符跟另一个 basic_string 下标 pos2 开始的 count2 个字符比较
template <class CharType, class CharTraits>
int basic_string<CharType, CharTraits>::
compare(size_type pos1, size_type count1, const basic_string& other,
        size_type pos2, size_type count2) const
{
  auto n1 = mystl::min(count1, size_ - pos1);
  auto n2 = mystl::min(count2, other.size_ - pos2);
  return compare_cstr(buffer_, n1, other.buffer_, n2);
}

// 跟一个字符串比较
template <class CharType, class CharTraits>
int basic_string<CharType, CharTraits>::
compare(const_pointer s) const
{
  auto n2 = char_traits::length(s);
  return compare_cstr(buffer_, size_, s, n2);
}

// 从下标 pos1 开始的 count1 个字符跟另一个字符串比较
template <class CharType, class CharTraits>
int basic_string<CharType, CharTraits>::
compare(size_type pos1, size_type count1, const_pointer s) const
{
  auto n1 = mystl::min(count1, size_ - pos1);
  auto n2 = char_traits::length(s);
  return compare_cstr(buffer_, n1, s, n2);
}

// 从下标 pos1 开始的 count1 个字符跟另一个字符串的前 count2 个字符比较
template <class CharType, class CharTraits>
int basic_string<CharType, CharTraits>::
compare(size_type pos1, size_type count1, const_pointer s, size_type count2) const
{
  auto n1 = mystl::min(count1, size_ - pos1);
  return compare_cstr(buffer_, n1, s, count2);
}

// 反转 basic_string
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
reverse() noexcept
{
  for (auto i = begin(), j = end(); i < j;)
  {
    mystl::iter_swap(i++, --j);//不管是什么对象（内置类型或者自定义的类）都可以进行交换操作
  }
}

// 交换两个 basic_string
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
swap(basic_string& rhs) noexcept
{
  if (this != &rhs)
  {//防止自交换，因为mystl::swap会现将lhs给转成右值转移给临时对象
    mystl::swap(buffer_, rhs.buffer_);
    mystl::swap(size_, rhs.size_);
    mystl::swap(cap_, rhs.cap_);
  }
}

// 从下标 pos 开始查找字符为 ch 的元素，若找到返回其下标，否则返回 npos
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type//返回一个size_type的下标
basic_string<CharType, CharTraits>::
find(value_type ch, size_type pos) const noexcept
{
  for (auto i = pos; i < size_; ++i)
  {
    if (*(buffer_ + i) == ch)
      return i;
  }
  return this->npos;
}

// 从下标 pos 开始查找字符串 str，若找到返回起始位置的下标，否则返回 npos
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find(const_pointer str, size_type pos) const noexcept
{
  const auto len = char_traits::length(str);//注意str是一个数组指针，因此需要专门针对指针进行操作
  if (len == 0)
    return pos;
  if (size_ - pos < len)//长度已经超过限制了
    return npos;
  const auto left = size_ - len;//最后一个可以比较的下标位置，提前限制的话就不需要在担心越界了
  for (auto i = pos; i <= left; ++i)
  {
    if (*(buffer_ + i) == *str)
    {
      size_type j = 1;
      for (; j < len; ++j)
      {//逐个比较
        if (*(buffer_ + i + j) != *(str + j))
          break;
      }
      if (j == len)//比较完成，找到了这个字符串，返回左下标
        return i;
    }
  }
  return npos;//否则返回npos
}

// 从下标 pos 开始查找字符串 str 的前 count 个字符，若找到返回起始位置的下标，否则返回 npos
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find(const_pointer str, size_type pos, size_type count) const noexcept
{
  if (count == 0)
    return pos;
  if (size_ - pos < count)
    return npos;
  const auto left = size_ - count;
  for (auto i = pos; i <= left; ++i)
  {
    if (*(buffer_ + i) == *str)
    {
      size_type j = 1;
      for (; j < count; ++j)
      {
        if (*(buffer_ + i + j) != *(str + j))
          break;
      }
      if (j == count)
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找字符串 str，若找到返回起始位置的下标，否则返回 npos
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find(const basic_string& str, size_type pos) const noexcept//上面字符串用指针表示，这里用string表示
{
  const size_type count = str.size_;//这里就不需要对指针操作了
  if (count == 0)
    return pos;
  if (size_ - pos < count)
    return npos;
  const auto left = size_ - count;
  for (auto i = pos; i <= left; ++i)
  {
    if (*(buffer_ + i) == str.front())
    {
      size_type j = 1;
      for (; j < count; ++j)
      {
        if (*(buffer_ + i + j) != str[j])
          break;
      }
      if (j == count)
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始反向查找值为 ch 的元素，与 find 类似
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
rfind(value_type ch, size_type pos) const noexcept
{
  if (pos >= size_)
    pos = size_ - 1;
  for (auto i = pos; i != 0; --i)//没有判断0位置，因为没办法判断，如果写成i>=0的话由于i是一个size_type,永远不会出现负数，那么循环会一直进行
                                 //写成i>0也可以，效果相同
  {
    if (*(buffer_ + i) == ch)
      return i;
  }
  return front() == ch ? 0 : npos;//单独判断0下标
}

// 从下标 pos 开始反向查找字符串 str，与 find 类似
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
rfind(const_pointer str, size_type pos) const noexcept
{
  if (pos >= size_)
    pos = size_ - 1;
  const size_type len = char_traits::length(str);
  switch (len)
  {
    case 0: 
      return pos;
    case 1: 
    {
      for (auto i = pos; i != 0; --i)
      {
        if (*(buffer_ + i) == *str)
          return i;
      }
      return front() == *str ? 0 : npos;
    }
    default:
    { // len >= 2
      for (auto i = pos; i >= len - 1; --i)//i是末尾字符下标
      {
        if (*(buffer_ + i) == *(str + len - 1))//这里是从后往前比较
        {
          size_type j = 1;
          for (; j < len; ++j)
          {
            if (*(buffer_ + i - j) != *(str + len - j - 1))//往前比较
              break;
          }
          if (j == len)
            return i - len + 1;//反向比较，因此返回的是末尾字符的下标
        }
      }//Switch语句的case里面都要有break或者return
      break;
    }
  }
  return npos;
}

// 从下标 pos 开始反向查找字符串 str 前 count 个字符，与 find 类似
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
rfind(const_pointer str, size_type pos, size_type count) const noexcept
{
  if (count == 0)
    return pos;
  if (pos >= size_)
    pos = size_ - 1;
  if (pos < count - 1)
    return npos;
  for (auto i = pos; i >= count - 1; --i)
  {
    if (*(buffer_ + i) == *(str + count - 1))
    {
      size_type j = 1;
      for (; j < count; ++j)
      {
        if (*(buffer_ + i - j) != *(str + count - j - 1))
          break;
      }
      if (j == count)
        return i - count + 1;
    }
  }
  return npos;
}

// 从下标 pos 开始反向查找字符串 str，与 find 类似
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
rfind(const basic_string& str, size_type pos) const noexcept
{
  const size_type count = str.size_;
  if (pos >= size_)
    pos = size_ - 1;
  if (count == 0)
    return pos;
  if (pos < count - 1)
    return npos;
  for (auto i = pos; i >= count - 1; --i)
  {
    if (*(buffer_ + i) == str[count - 1])
    {
      size_type j = 1;
      for (; j < count; ++j)
      {
        if (*(buffer_ + i - j) != str[count - j - 1])
          break;
      }
      if (j == count)
        return i - count + 1;
    }
  }
  return npos;
}

// 从下标 pos 开始查找 ch 出现的第一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_first_of(value_type ch, size_type pos) const noexcept
{
  for (auto i = pos; i < size_; ++i)
  {
    if (*(buffer_ + i) == ch)
      return i;
  }
  return npos;
}

// 从下标 pos 开始查找字符串 s 其中的一个字符出现的第一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_first_of(const_pointer s, size_type pos) const noexcept
{
  const size_type len = char_traits::length(s);
  for (auto i = pos; i < size_; ++i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < len; ++j)
    {
      if (ch == *(s + j))
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找字符串 s 的[0:count)个其中的一个字符出现的第一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_first_of(const_pointer s, size_type pos, size_type count) const noexcept
{
  for (auto i = pos; i < size_; ++i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < count; ++j)
    {
      if (ch == *(s + j))
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找字符串 str 其中一个字符出现的第一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_first_of(const basic_string& str, size_type pos) const noexcept
{
  for (auto i = pos; i < size_; ++i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < str.size_; ++j)
    {
      if (ch == str[j])
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与 ch 不相等的第一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_first_not_of(value_type ch, size_type pos) const noexcept
{
  for (auto i = pos; i < size_; ++i)
  {
    if (*(buffer_ + i) != ch)
      return i;
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 s 其中一个字符不相等的第一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_first_not_of(const_pointer s, size_type pos) const noexcept
{
  const size_type len = char_traits::length(s);
  for (auto i = pos; i < size_; ++i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < len; ++j)
    {
      if (ch != *(s + j))
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 s 前 count 个字符中不相等的第一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_first_not_of(const_pointer s, size_type pos, size_type count) const noexcept
{
  for (auto i = pos; i < size_; ++i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < count; ++j)
    {
      if (ch != *(s + j))
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 str 的字符中不相等的第一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_first_not_of(const basic_string& str, size_type pos) const noexcept
{
  for (auto i = pos; i < size_; ++i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < str.size_; ++j)
    {
      if (ch != str[j])
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与 ch 相等的最后一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_last_of(value_type ch, size_type pos) const noexcept//和rfind差不多
{
  for (auto i = size_ - 1; i >= pos; --i)
  {
    if (*(buffer_ + i) == ch)
      return i;
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 s 其中一个字符相等的最后一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_last_of(const_pointer s, size_type pos) const noexcept
{
  const size_type len = char_traits::length(s);
  for (auto i = size_ - 1; i >= pos; --i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < len; ++j)
    {
      if (ch == *(s + j))
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 s 前 count 个字符中相等的最后一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_last_of(const_pointer s, size_type pos, size_type count) const noexcept
{
  for (auto i = size_ - 1; i >= pos; --i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < count; ++j)
    {
      if (ch == *(s + j))
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 str 字符中相等的最后一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_last_of(const basic_string& str, size_type pos) const noexcept
{
  for (auto i = size_ - 1; i >= pos; --i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < str.size_; ++j)
    {
      if (ch == str[j])
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与 ch 字符不相等的最后一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_last_not_of(value_type ch, size_type pos) const noexcept
{
  for (auto i = size_ - 1; i >= pos; --i)
  {
    if (*(buffer_ + i) != ch)
      return i;
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 s 的字符中不相等的最后一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_last_not_of(const_pointer s, size_type pos) const noexcept
{
  const size_type len = char_traits::length(s);
  for (auto i = size_ - 1; i >= pos; --i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < len; ++j)
    {
      if (ch != *(s + j))
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 s 前 count 个字符中不相等的最后一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_last_not_of(const_pointer s, size_type pos, size_type count) const noexcept
{
  for (auto i = size_ - 1; i >= pos; --i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < count; ++j)
    {
      if (ch != *(s + j))
        return i;
    }
  }
  return npos;
}

// 从下标 pos 开始查找与字符串 str 字符中不相等的最后一个位置
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
find_last_not_of(const basic_string& str, size_type pos) const noexcept
{
  for (auto i = size_ - 1; i >= pos; --i)
  {
    value_type ch = *(buffer_ + i);
    for (size_type j = 0; j < str.size_; ++j)
    {
      if (ch != str[j])
        return i;
    }
  }
  return npos;
}

// 返回从下标 pos 开始字符为 ch 的元素出现的次数
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::size_type
basic_string<CharType, CharTraits>::
count(value_type ch, size_type pos) const noexcept
{
  size_type n = 0;
  for (auto i = pos; i < size_; ++i)
  {
    if (*(buffer_ + i) == ch)
      ++n;
  }
  return n;
}

/*****************************************************************************************/
// helper function

// 尝试初始化一段 buffer，若分配失败则忽略，不会抛出异常
template <class CharType, class CharTraits>//模板类的成员函数必须要写成这样
void basic_string<CharType, CharTraits>::
try_init() noexcept
{
  try
  {
    buffer_ = data_allocator::allocate(static_cast<size_type>(STRING_INIT_SIZE));//尝试分配32字节大小的空间
    size_ = 0;//分配成功为什么size和cap还是0？因为调用这个函数只有是默认构造函数里面，没有任何初始化工作
    cap_ = 0;
  }
  catch (...)
  {
    buffer_ = nullptr;
    size_ = 0;
    cap_ = 0;
    // no throw
  }
}

// fill_init 函数
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
fill_init(size_type n, value_type ch)
{
  const auto init_size = mystl::max(static_cast<size_type>(STRING_INIT_SIZE), n + 1);//最后一个空间用来存放\0
  //如果申请的空间小于32字节，那么仍然申请32字节大小的空间
  buffer_ = data_allocator::allocate(init_size);
  char_traits::fill(buffer_, ch, n);//调用的是20-210那些char_traits中的fill函数
  size_ = n;//size是实际容量，而cap是总容量
  cap_ = init_size;
}

// copy_init 函数
template <class CharType, class CharTraits>//函数在模板类外定义，需要加上模板参数
template <class Iter>//模板构造函数的第二个模板参数不一定存在，所以这里没有写
void basic_string<CharType, CharTraits>::
copy_init(Iter first, Iter last, mystl::input_iterator_tag)
{
  size_type n = mystl::distance(first, last);
  const auto init_size = mystl::max(static_cast<size_type>(STRING_INIT_SIZE), n + 1);
  try
  {
    buffer_ = data_allocator::allocate(init_size);//这里只是申请内存，有可能会有异常，所以要写到try里面
    size_ = n;//为什么这里就不是0了？因为这里初始化了，需要往内存里构造对象
    cap_ = init_size;
  }
  catch (...)
  {
    buffer_ = nullptr;
    size_ = 0;
    cap_ = 0;
    throw;//这里会抛出异常
  }
  for (; n > 0; --n, ++first)//这里是什么意思？申请内存再把
    append(*first);//拷贝过来初始化？这里调用的是哪个函数？似乎是没有实现输入迭代器版本的append
}

template <class CharType, class CharTraits>
template <class Iter>
void basic_string<CharType, CharTraits>::
copy_init(Iter first, Iter last, mystl::forward_iterator_tag)//常用的是这个前向迭代器版本的
{
  const size_type n = mystl::distance(first, last);
  const auto init_size = mystl::max(static_cast<size_type>(STRING_INIT_SIZE), n + 1);
  try
  {
    buffer_ = data_allocator::allocate(init_size);
    size_ = n;
    cap_ = init_size;
    mystl::uninitialized_copy(first, last, buffer_);
  }
  catch (...)
  {
    buffer_ = nullptr;
    size_ = 0;
    cap_ = 0;
    throw;//这里不做处理，等上层函数处理
  }
}

// init_from 函数
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
init_from(const_pointer src, size_type pos, size_type count)
{//可以给一个常量形参传入非常量，就比如这里我们给const_pointer传入的是other.buffer_，只是一个普通的指针
    //从别的basic_string的内存中拷贝字符，src就是原地址，pos是从原地址的哪个字符开始拷贝，count是拷贝字符的个数
  const auto init_size = mystl::max(static_cast<size_type>(STRING_INIT_SIZE), count + 1);
  buffer_ = data_allocator::allocate(init_size);
  //allocate返回的指针指向开始(最低的字节地址)分配的存储地址
  char_traits::copy(buffer_, src + pos, count);//从src+pos的地址开始，拷贝count个字符到buffer_这块新申请的内存上
  size_ = count;//实际大小是count个字符
  cap_ = init_size;//容量就是申请的内存大小
}

// destroy_buffer 函数
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
destroy_buffer()
{
  if (buffer_ != nullptr)
  {
    data_allocator::deallocate(buffer_, cap_);//销毁内存空间
    buffer_ = nullptr;
    size_ = 0;
    cap_ = 0;
  }
}

// to_raw_pointer 函数
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::const_pointer//返回值是一个类型
basic_string<CharType, CharTraits>::
to_raw_pointer() const
{
  *(buffer_ + size_) = value_type();//在末尾的位置构造一个默认对象（就是字符数组末尾默认的/0），如果没有这个构造，转换成指针后，如果用指针去访问内存，那么最后一个内存
                                    //的地方由于没有构造结束符/0。那么会越界访问出错，而上层迭代器就不需要担心这个，因为封装以后保证迭代器
                                    //不会越界访问，指针没有约束就无法保证
  return buffer_;
}

// reinsert 函数，只用来缩小空间？没找到其他用法，为什么不直接合并？
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
reinsert(size_type size)
{
  auto new_buffer = data_allocator::allocate(size);//申请新内存
  try
  {
    char_traits::move(new_buffer, buffer_, size);//尝试转移
  }
  catch (...)
  {
    data_allocator::deallocate(new_buffer);//抛出任何异常都把新内存释放掉，然后结束此函数运行
  }
  buffer_ = new_buffer;//没有发生异常就把新地址换过去，数据已经在try里面转移了
  size_ = size;
  cap_ = size;
}

// append_range，末尾追加一段 [first, last) 内的字符
template <class CharType, class CharTraits>//模板类的模板参数
template <class Iter>//模板类的模板函数的模板参数
basic_string<CharType, CharTraits>&//返回值，basic_string<CharType, CharTraits>用来实例化模板，生成类
basic_string<CharType, CharTraits>:://表明这个函数是属于哪个类的成员函数
append_range(Iter first, Iter last)
{
  const size_type n = mystl::distance(first, last);
  THROW_LENGTH_ERROR_IF(size_ > max_size() - n,
                        "basic_string<Char, Tratis>'s size too big");
  if (cap_ - size_ < n)
  {
    reallocate(n);
  }
  mystl::uninitialized_copy_n(first, n, buffer_ + size_);
  size_ += n;
  return *this;
}

template <class CharType, class CharTraits>
int basic_string<CharType, CharTraits>::
compare_cstr(const_pointer s1, size_type n1, const_pointer s2, size_type n2) const
{
  auto rlen = mystl::min(n1, n2);
  auto res = char_traits::compare(s1, s2, rlen);//只比较rlen这么长的字符
  if (res != 0) return res;
  if (n1 < n2) return -1;//等于0说明上面比较的字符都是相同的，那么谁短谁就在前面
  if (n1 > n2) return 1;
  return 0;
}

// 把 first 开始的 count1 个字符替换成 str 开始的 count2 个字符
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>& 
basic_string<CharType, CharTraits>::
replace_cstr(const_iterator first, size_type count1, const_pointer str, size_type count2)
{
  if (static_cast<size_type>(cend() - first) < count1)
  {//原本字符串长度没有count1个字符，则不能够替换
    count1 = cend() - first;
  }
  if (count1 < count2)
  {
    const size_type add = count2 - count1;//需要补充的字符个数
    THROW_LENGTH_ERROR_IF(size_ > max_size() - add,//补充的字符太多以至于超过了最大长度
                          "basic_string<Char, Traits>'s size too big");
    if (size_ > cap_ - add)
    {//补充的字符太多以至于超过了字符串容量，那么就需要重新申请一块内存
      reallocate(add);
    }
    pointer r = const_cast<pointer>(first);//去掉const属性，因为下面要修改
    char_traits::move(r + count2, first + count1, end() - (first + count1));
    char_traits::copy(r, str, count2);
    size_ += add;
  }
  else
  {
    pointer r = const_cast<pointer>(first);
    char_traits::move(r + count2, first + count1, end() - (first + count1));
    char_traits::copy(r, str, count2);
    size_ -= (count1 - count2);
  }
  return *this;
}

// 把 first 开始的 count1 个字符替换成 count2 个 ch 字符
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>&
basic_string<CharType, CharTraits>::
replace_fill(const_iterator first, size_type count1, size_type count2, value_type ch)
{
  if (static_cast<size_type>(cend() - first) < count1)
  {
    count1 = cend() - first;
  }
  if (count1 < count2)
  {
    const size_type add = count2 - count1;
    THROW_LENGTH_ERROR_IF(size_ > max_size() - add,
                          "basic_string<Char, Traits>'s size too big");
    if (size_ > cap_ - add)
    {
      reallocate(add);
    }
    pointer r = const_cast<pointer>(first);
    char_traits::move(r + count2, first + count1, end() - (first + count1));
    char_traits::fill(r, ch, count2);
    size_ += add;
  }
  else
  {
    pointer r = const_cast<pointer>(first);
    char_traits::move(r + count2, first + count1, end() - (first + count1));
    char_traits::fill(r, ch, count2);
    size_ -= (count1 - count2);
  }
  return *this;
}

// 把 [first, last) 的字符替换成 [first2, last2)
template <class CharType, class CharTraits>
template <class Iter>
basic_string<CharType, CharTraits>&
basic_string<CharType, CharTraits>::
replace_copy(const_iterator first, const_iterator last, Iter first2, Iter last2)
{
  size_type len1 = last - first;
  size_type len2 = last2 - first2;
  if (len1 < len2)
  {
    const size_type add = len2 - len1;
    THROW_LENGTH_ERROR_IF(size_ > max_size() - add,
                          "basic_string<Char, Traits>'s size too big");
    if (size_ > cap_ - add)
    {
      reallocate(add);
    }
    pointer r = const_cast<pointer>(first);
    char_traits::move(r + len2, first + len1, end() - (first + len1));
    char_traits::copy(r, first2, len2);
    size_ += add;
  }
  else
  {
    pointer r = const_cast<pointer>(first);
    char_traits::move(r + len2, first + len1, end() - (first + len1));
    char_traits::copy(r, first2, len2);
    size_ -= (len1 - len2);
  }
  return *this;
}

// reallocate 函数，重新申请一块内存
template <class CharType, class CharTraits>
void basic_string<CharType, CharTraits>::
reallocate(size_type need)
{
  const auto new_cap = mystl::max(cap_ + need, cap_ + (cap_ >> 1));
  auto new_buffer = data_allocator::allocate(new_cap);//新内存的地址
  char_traits::move(new_buffer, buffer_, size_);//转移数据
  data_allocator::deallocate(buffer_);//释放原有内存
  buffer_ = new_buffer;
  cap_ = new_cap;
}

// reallocate_and_fill 函数
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::iterator
basic_string<CharType, CharTraits>::
reallocate_and_fill(iterator pos, size_type n, value_type ch)
{//重新申请内存，并在pos的地方插入n个ch
  const auto r = pos - buffer_;
  const auto old_cap = cap_;
  const auto new_cap = mystl::max(old_cap + n, old_cap + (old_cap >> 1));//如果n小于old_cap的一半，则直接申请一半的内存
  auto new_buffer = data_allocator::allocate(new_cap);
  auto e1 = char_traits::move(new_buffer, buffer_, r) + r;//move把原始数据的r个元素移动到新内存，返回的是新地址的首迭代器，再加上r
  auto e2 = char_traits::fill(e1, ch, n) + n;//然后填充n个ch到新内存的末尾
  char_traits::move(e2, buffer_ + r, size_ - r);//再把原始数据剩余的元素移动到新内存
  data_allocator::deallocate(buffer_, old_cap);//析构原内存
  buffer_ = new_buffer;
  size_ += n;
  cap_ = new_cap;
  return buffer_ + r;//返回的是插入字符的首地址
}

// reallocate_and_copy 函数
template <class CharType, class CharTraits>
typename basic_string<CharType, CharTraits>::iterator
basic_string<CharType, CharTraits>::
reallocate_and_copy(iterator pos, const_iterator first, const_iterator last)
{//重新申请内存，并在pos的地方拷贝[first,last)
  const auto r = pos - buffer_;
  const auto old_cap = cap_;
  const size_type n = mystl::distance(first, last);
  const auto new_cap = mystl::max(old_cap + n, old_cap + (old_cap >> 1));
  auto new_buffer = data_allocator::allocate(new_cap);
  auto e1 = char_traits::move(new_buffer, buffer_, r) + r;
  auto e2 = mystl::uninitialized_copy_n(first, n, e1) + n;
  char_traits::move(e2, buffer_ + r, size_ - r);
  data_allocator::deallocate(buffer_, old_cap);
  buffer_ = new_buffer;
  size_ += n;
  cap_ = new_cap;
  return buffer_ + r;
}

/*****************************************************************************************/
// 重载全局操作符

// 重载 operator+，这是函数重载，不是成员函数，string+string
template <class CharType, class CharTraits>//函数模板参数
basic_string<CharType, CharTraits>//返回值是basic_string的对象，不是指针、引用
operator+(const basic_string<CharType, CharTraits>& lhs, 
          const basic_string<CharType, CharTraits>& rhs)//两个参数
{
  basic_string<CharType, CharTraits> tmp(lhs);//拷贝构造一个临时对象
  tmp.append(rhs);//加到后面
  return tmp;//临时对象不能返回指针、引用
}

//const char数组 + string
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>//仍然返回一个string
operator+(const CharType* lhs, const basic_string<CharType, CharTraits>& rhs)
{
  basic_string<CharType, CharTraits> tmp(lhs);//注意lhs是一个数组的首地址，这里调用的是340行的构造函数
  tmp.append(rhs);
  return tmp;
}

//char + string
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(CharType ch, const basic_string<CharType, CharTraits>& rhs)
{
  basic_string<CharType, CharTraits> tmp(1, ch);//注意ch是一个字符，这里调用的是323行的构造函数
  tmp.append(rhs);
  return tmp;
}

//string + const char数组
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(const basic_string<CharType, CharTraits>& lhs, const CharType* rhs)
{
  basic_string<CharType, CharTraits> tmp(lhs);//拷贝构造
  tmp.append(rhs);//注意rhs是指针，调用的是515行的append
  return tmp;
}

//string + char
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(const basic_string<CharType, CharTraits>& lhs, CharType ch)
{
  basic_string<CharType, CharTraits> tmp(lhs);
  tmp.append(1, ch);
  return tmp;
}

//右值string+string，调用形式：res=string("abcde")+str1，前面是临时对象，后面是左值
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(basic_string<CharType, CharTraits>&& lhs,
          const basic_string<CharType, CharTraits>& rhs)
{
  basic_string<CharType, CharTraits> tmp(mystl::move(lhs));//转移构造
  tmp.append(rhs);
  return tmp;
}

//string+右值string，调用形式：res=str1+string("abcde")，前面是左值，后面是临时对象
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(const basic_string<CharType, CharTraits>& lhs,
          basic_string<CharType, CharTraits>&& rhs)
{
  basic_string<CharType, CharTraits> tmp(mystl::move(rhs));
  tmp.insert(tmp.begin(), lhs.begin(), lhs.end());//这里用的就是插入了，可能是为了避免拷贝构造
  return tmp;
}

//右值string+右值string，调用形式：res=string("ab")+string("cde")
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(basic_string<CharType, CharTraits>&& lhs,
          basic_string<CharType, CharTraits>&& rhs)
{
  basic_string<CharType, CharTraits> tmp(mystl::move(lhs));
  tmp.append(rhs);
  return tmp;
}

//const char数组+右值string，调用形式：res="ab"+string("cde")
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(const CharType* lhs, basic_string<CharType, CharTraits>&& rhs)
{
  basic_string<CharType, CharTraits> tmp(mystl::move(rhs));
  tmp.insert(tmp.begin(), lhs, lhs + char_traits<CharType>::length(lhs));
  return tmp;
}

//char+右值string，调用形式：res='a'+string("cde")
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(CharType ch, basic_string<CharType, CharTraits>&& rhs)
{
  basic_string<CharType, CharTraits> tmp(mystl::move(rhs));
  tmp.insert(tmp.begin(), ch);
  return tmp;
}

//右值string+const char数组，调用形式：res=string("cde")+"ab"
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(basic_string<CharType, CharTraits>&& lhs, const CharType* rhs)
{
  basic_string<CharType, CharTraits> tmp(mystl::move(lhs));
  tmp.append(rhs);
  return tmp;
}

//右值string+char，调用形式：res=string("cde")+'a'
template <class CharType, class CharTraits>
basic_string<CharType, CharTraits>
operator+(basic_string<CharType, CharTraits>&& lhs, CharType ch)
{
  basic_string<CharType, CharTraits> tmp(mystl::move(lhs));
  tmp.append(1, ch);
  return tmp;
}

// 重载比较操作符
template <class CharType, class CharTraits>
bool operator==(const basic_string<CharType, CharTraits>& lhs,
                const basic_string<CharType, CharTraits>& rhs)
{
  return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}

template <class CharType, class CharTraits>
bool operator!=(const basic_string<CharType, CharTraits>& lhs,
                const basic_string<CharType, CharTraits>& rhs)
{
  return lhs.size() != rhs.size() || lhs.compare(rhs) != 0;
}

template <class CharType, class CharTraits>
bool operator<(const basic_string<CharType, CharTraits>& lhs,
               const basic_string<CharType, CharTraits>& rhs)
{
  return lhs.compare(rhs) < 0;
}

template <class CharType, class CharTraits>
bool operator<=(const basic_string<CharType, CharTraits>& lhs,
                const basic_string<CharType, CharTraits>& rhs)
{
  return lhs.compare(rhs) <= 0;
}

template <class CharType, class CharTraits>
bool operator>(const basic_string<CharType, CharTraits>& lhs,
               const basic_string<CharType, CharTraits>& rhs)
{
  return lhs.compare(rhs) > 0;
}

template <class CharType, class CharTraits>
bool operator>=(const basic_string<CharType, CharTraits>& lhs,
                const basic_string<CharType, CharTraits>& rhs)
{
  return lhs.compare(rhs) >= 0;
}

// 重载 mystl 的 swap，也就是当调用swap(str1,str2)的时候，就会调用这个函数
template <class CharType, class CharTraits>
void swap(basic_string<CharType, CharTraits>& lhs,
          basic_string<CharType, CharTraits>& rhs) noexcept
{
  lhs.swap(rhs);
}

// 特化 mystl::hash
template <class CharType, class CharTraits>
struct hash<basic_string<CharType, CharTraits>>
{
  size_t operator()(const basic_string<CharType, CharTraits>& str)
  {
    return bitwise_hash((const unsigned char*)str.c_str(),
                        str.size() * sizeof(CharType));
  }
};//这是一个特例化的类，后面要加上分号

} // namespace mystl
#endif // !MYTINYSTL_BASIC_STRING_H_

