#ifndef MYTINYSTL_DEQUE_H_
#define MYTINYSTL_DEQUE_H_

// 这个头文件包含了一个模板类 deque
// deque: 双端队列

// notes:
//
// 异常保证：
// mystl::deque<T> 满足基本异常保证，部分函数无异常保证，并对以下等函数做强异常安全保证：
//   * emplace_front
//   * emplace_back
//   * emplace
//   * push_front
//   * push_back
//   * insert

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"

namespace mystl
{
//这段预编译代码的功能是：如果标识符 max 已经用宏定义进行过定义，那么执行下面内容：
//在编译输出窗口打印文本 “#undefing marco max”
//删除事先定义的宏定义 max(#undef max)，会在下面重新定义？

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif // min

// deque map 初始化的大小
#ifndef DEQUE_MAP_INIT_SIZE
#define DEQUE_MAP_INIT_SIZE 8
#endif

template <class T>
struct deque_buf_size
{
  static constexpr size_t value = sizeof(T) < 256 ? 4096 / sizeof(T) : 16;//这是一个静态常量，最小值是16，不能用全局吗？
};

// deque 的迭代器设计，注意和string的区别，string的迭代器实际上就是简单地把元素指针重命名为 迭代器，而这里迭代器进行了许多封装，
//所以容器deque的迭代器要比string的迭代器复杂得多，也支持更多的操作，比如自增，自减之类的，string的迭代器只能像指针那样操作
template <class T, class Ref, class Ptr>
struct deque_iterator : public iterator<random_access_iterator_tag, T>//deque的迭代器是随机迭代器，所以会继承
{
  typedef deque_iterator<T, T&, T*>             iterator;//这里声明别名，deque_iterator<T, T&, T*>在模板类内来说是一个类名（相当于实例化模板了），给这个类起个别名叫iterator
  typedef deque_iterator<T, const T&, const T*> const_iterator;//常量类的别名
  typedef deque_iterator                        self;//感觉和上面的是不是一样的？上面的好像是特例化出来的一个类，更具体一些？

  typedef T            value_type;
  typedef Ptr          pointer;
  typedef Ref          reference;
  typedef size_t       size_type;
  typedef ptrdiff_t    difference_type;//两个指针相间的类型
  typedef T*           value_pointer;//指向底层数据的指针，也就是缓冲区里面保存的数据的指针
  typedef T**          map_pointer;//指向指针的指针，也就是指向缓冲区的指针，用来找到缓冲区的位置，map中控里面保存的是value_pointer，
                                    //而迭代器需要保存一个指向map中控里面value_pointer的指针，用来找到map中控里面的value_pointer，从而找到缓冲区的位置
  //https://blog.csdn.net/oneNYT/article/details/107724892
  static const size_type buffer_size = deque_buf_size<T>::value;//只读静态常量成员，所有对象共享，且不能修改，所有迭代器指向的缓冲区大小都是buffer_size

  // 迭代器所含成员数据
  value_pointer cur;    // 指向所在缓冲区的当前元素，这里直接用的指针，迭代器为什么要这么多指针？因为deque的内存映像和vector不一样，是由不连续的缓冲区组成的，需要知道缓冲区的实际位置。
  value_pointer first;  // 指向所在缓冲区的头部
  value_pointer last;   // 指向所在缓冲区的尾部
  map_pointer   node;   // 指向的是map中控区中的value_pointer

  // 构造、复制、移动函数
  deque_iterator() noexcept
    :cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}

  deque_iterator(value_pointer v, map_pointer n)//用另一个迭代器来初始化，v直接指向缓冲区的当前元素，n指向的是map中控，里面保存的是缓冲区的首地址
                                                //而first代表的就是缓冲区的首地址，所以对n取值以后就等于first
    :cur(v), first(*n), last(*n + buffer_size), node(n) {}//似乎只是简单的复制指针？而且这个加上buffer_size是什么意思？

  deque_iterator(const iterator& rhs)//拷贝构造，这里用的是const iterator，可以理解为const pointer，即rhs本身是一个只读量，那么rhs里面的成员也都是只读量，因此不能改变
    :cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node)
  {
      //rhs.first = rhs.first;//所以这里会报错，只读量对象的成员都是只读量，无法改变
  }
  deque_iterator(iterator&& rhs) noexcept//转移构造，绝对无异常
    :cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node)
  {
    rhs.cur = nullptr;
    rhs.first = nullptr;
    rhs.last = nullptr;
    rhs.node = nullptr;
  }

  deque_iterator(const const_iterator& rhs)//什么鬼意思？除了迭代器指向的元素是个只读量，迭代器本身也是个只读量，也就是rhs不能改变引用的对象（其实引用本身就不可改变），也不能改变自己所引用的对象
      //iterator 可遍历，可改变所指元素
      //const_iterator 可遍历，不可改变所指元素，他修饰的是迭代器所指向元素本身（因为迭代器就是指针）
      //const iterator 不可遍历，可改变所指元素，他修饰的是迭代器本身（就是指针本身），可以理解成iterator是一个对象，
      //    和int，double一样，那么const iterator说明变量（迭代器本身）是一个只读量，无法给他重新写入值（也就是将迭代器重新指向是不允许的）
      //const_iterator 主要是在容器被定义成常量、或者非常量容器但不想改变元素值的情况下使用的，
      //而且容器被定义成常量之后，它返回的迭代器只能是const_iterator
      //这个构造函数只有当rhs是一个只读容器中的迭代器的时候才会调用
    :cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node)
  {
      //rhs.first = rhs.first;//这里的限制是因为最前面的const，
  }

  self& operator=(const iterator& rhs)
  {
    if (this != &rhs)
    {
      cur = rhs.cur;
      first = rhs.first;
      last = rhs.last;
      node = rhs.node;
    }
    return *this;
  }

  // 转到另一个缓冲区
  void set_node(map_pointer new_node)
  {//只要有new_node就能找到新的缓冲区位置，这里不是新开辟buffer，只是转换一下所指向的buffer
    node = new_node;
    first = *new_node;
    last = first + buffer_size;//所有buffer的大小都是固定的buffer_size
  }

  // 重载运算符
  reference operator*()  const { return *cur; }//对迭代器取值，和指针是一样的，取出来以后就是一个普通对象，比如int，double
  pointer   operator->() const { return cur; }//对于普通对象int，double来说，肯定不需要进行->的操作，因为他们不存在方法，无法调用，而对于
                                              //string或者自定义的类来说，则需要使用其中的方法，所以迭代器定义了->

  difference_type operator-(const self& x) const//迭代器相减,用self或者iterator是一样的
  {//两个迭代器可能指向的不是同一个buffer，所以要考虑node之间的距离，注意node - x.node是指针相减，得到的是相隔缓冲区的数量，如果指向的是同一个
      //buffer的话，那么node - x.node就等于0，假如x指向的是第一个缓冲区，self指向的是第三个缓冲区，那么node - x.node就等于2
      //然后，加上self多出来的那一段，再减去多算 x 的那一段（上面得到的2里面有一部分是多算的）
    return static_cast<difference_type>(buffer_size) * (node - x.node)
      + (cur - first) - (x.cur - x.first);
  }

  self& operator++()
  {//这是前置递增运算符
    ++cur;//先递增cur
    if (cur == last)
    { // 如果到达缓冲区的尾，那么这里不能存对象，需要换到下一个buffer去，也就是让当前这个迭代器指向下一个buffer
      set_node(node + 1);//换到下一个buffer
      cur = first;
    }
    return *this;//返回自身引用，如果返回值的话不能出现在等号左边，显然不符合迭代器的定义
  }
  self operator++(int)
  {//这是后置递增运算符，用int参数来做区分
    self tmp = *this;//先保存一下自身
    ++*this;//调用前置递增运算符自增
    return tmp;//返回旧值，注意这里返回的是值，而不是引用，因此前置递增运算符可以出现在等号左边，而后置递增则不行，不能给值赋值
  }

  self& operator--()
  {//同理
    if (cur == first)
    { // 如果到达缓冲区的头
      set_node(node - 1);
      cur = last;
    }
    --cur;
    return *this;
  }
  self operator--(int)
  {
    self tmp = *this;
    --*this;
    return tmp;
  }

  self& operator+=(difference_type n)//n是可以小于0的，所以下面要判断
  {//随机迭代器可以随意加减
    const auto offset = n + (cur - first);
    if (offset >= 0 && offset < static_cast<difference_type>(buffer_size))
    { // 仍在当前缓冲区
      cur += n;
    }
    else
    { // 要跳到其他的缓冲区
      const auto node_offset = offset > 0
        ? offset / static_cast<difference_type>(buffer_size)//首先看看需要跳过多少个buffer
        : -static_cast<difference_type>((-offset - 1) / buffer_size) - 1;
      set_node(node + node_offset);
      cur = first + (offset - node_offset * static_cast<difference_type>(buffer_size));
    }
    return *this;
  }
  self operator+(difference_type n) const
  {//返回一个临时对象
    self tmp = *this;
    return tmp += n;
  }
  self& operator-=(difference_type n)
  {
    return *this += -n;
  }
  self operator-(difference_type n) const
  {
    self tmp = *this;
    return tmp -= n;
  }

  reference operator[](difference_type n) const { return *(*this + n); }//迭代器有[]操作吗？这是啥意思

  // 重载比较操作符
  bool operator==(const self& rhs) const { return cur == rhs.cur; }
  bool operator< (const self& rhs) const
  { return node == rhs.node ? (cur < rhs.cur) : (node < rhs.node); }//等于比较cur，不等于比较node
  bool operator!=(const self& rhs) const { return !(*this == rhs); }//为什么不直接用！=
  bool operator> (const self& rhs) const { return rhs < *this; }//仍然是委托的
  bool operator<=(const self& rhs) const { return !(rhs < *this); }
  bool operator>=(const self& rhs) const { return !(*this < rhs); }
};

// 模板类 deque
// 模板参数代表数据类型
template <class T>
class deque
{
public:
  // deque 的型别定义
  typedef mystl::allocator<T>                      allocator_type;//申请内存空间，也就是上面说的buffer
  typedef mystl::allocator<T>                      data_allocator;
  typedef mystl::allocator<T*>                     map_allocator;//map中控，里面存的是指针，也是一段内存空间

  typedef typename allocator_type::value_type      value_type;//其实就是T，为什么不直接用呢？
  typedef typename allocator_type::pointer         pointer;//这个就是map_allocator里面存的东西，指向buffer数据的指针
  typedef typename allocator_type::const_pointer   const_pointer;
  typedef typename allocator_type::reference       reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type       size_type;
  typedef typename allocator_type::difference_type difference_type;
  typedef pointer*                                 map_pointer;//这个是指向map中控map_allocator的指针，就是T**，用来找到map中控
  typedef const_pointer*                           const_map_pointer;//只读deque的map中控里面的指针不能改变，也就是这个指针所指向的对象不能
                                                                     //改变，因此定义成const_pointer,而const pointer是修饰这个指针是个只读量
                                                                     //无法改变指向，而可以修改对象的值

  typedef deque_iterator<T, T&, T*>                iterator;//上面那个迭代器类
  typedef deque_iterator<T, const T&, const T*>    const_iterator;
  typedef mystl::reverse_iterator<iterator>        reverse_iterator;
  typedef mystl::reverse_iterator<const_iterator>  const_reverse_iterator;

  allocator_type get_allocator() { return allocator_type(); }//申请一个对象大小的空间，并返回这个空间对象

  static const size_type buffer_size = deque_buf_size<T>::value;//buffer大小，和迭代器那个定义是一样的

private:
  // 用以下四个数据来表现一个 deque
  iterator       begin_;     // 指向第一个节点
  iterator       end_;       // 指向最后一个结点
  map_pointer    map_;       // 指向一块 map，map 中的每个元素都是一个指针，指向一个缓冲区
  size_type      map_size_;  // map 内指针的数目

public:
  // 构造、复制、移动、析构函数

  deque()
  { fill_init(0, value_type()); }

  explicit deque(size_type n)
  { fill_init(n, value_type()); }

  deque(size_type n, const value_type& value)
  { fill_init(n, value); }

  template <class IIter, typename std::enable_if<
    mystl::is_input_iterator<IIter>::value, int>::type = 0>
  deque(IIter first, IIter last)
  { copy_init(first, last, iterator_category(first)); }

  deque(std::initializer_list<value_type> ilist)
  {
    copy_init(ilist.begin(), ilist.end(), mystl::forward_iterator_tag());
  }

  deque(const deque& rhs)
  {
    copy_init(rhs.begin(), rhs.end(), mystl::forward_iterator_tag());
  }
  deque(deque&& rhs) noexcept
    :begin_(mystl::move(rhs.begin_)),
    end_(mystl::move(rhs.end_)),
    map_(rhs.map_),
    map_size_(rhs.map_size_)
  {
    rhs.map_ = nullptr;
    rhs.map_size_ = 0;
  }

  deque& operator=(const deque& rhs);
  deque& operator=(deque&& rhs);

  deque& operator=(std::initializer_list<value_type> ilist)
  {
    deque tmp(ilist);
    swap(tmp);
    return *this;
  }

  ~deque()
  {
    if (map_ != nullptr)
    {
      clear();
      data_allocator::deallocate(*begin_.node, buffer_size);
      *begin_.node = nullptr;
      map_allocator::deallocate(map_, map_size_);
      map_ = nullptr;
    }
  }

public:
  // 迭代器相关操作

  iterator               begin()         noexcept
  { return begin_; }
  const_iterator         begin()   const noexcept
  { return begin_; }
  iterator               end()           noexcept
  { return end_; }
  const_iterator         end()     const noexcept
  { return end_; }

  reverse_iterator       rbegin()        noexcept
  { return reverse_iterator(end()); }
  const_reverse_iterator rbegin()  const noexcept
  { return reverse_iterator(end()); }
  reverse_iterator       rend()          noexcept
  { return reverse_iterator(begin()); }
  const_reverse_iterator rend()    const noexcept
  { return reverse_iterator(begin()); }

  const_iterator         cbegin()  const noexcept
  { return begin(); }
  const_iterator         cend()    const noexcept
  { return end(); }
  const_reverse_iterator crbegin() const noexcept
  { return rbegin(); }
  const_reverse_iterator crend()   const noexcept
  { return rend(); }

  // 容量相关操作

  bool      empty()    const noexcept  { return begin() == end(); }
  size_type size()     const noexcept  { return end_ - begin_; }
  size_type max_size() const noexcept  { return static_cast<size_type>(-1); }
  void      resize(size_type new_size) { resize(new_size, value_type()); }
  void      resize(size_type new_size, const value_type& value);
  void      shrink_to_fit() noexcept;

  // 访问元素相关操作 
  reference       operator[](size_type n)
  {
    MYSTL_DEBUG(n < size());
    return begin_[n];
  }
  const_reference operator[](size_type n) const
  {
    MYSTL_DEBUG(n < size());
    return begin_[n];
  }

  reference       at(size_type n)      
  { 
    THROW_OUT_OF_RANGE_IF(!(n < size()), "deque<T>::at() subscript out of range");
    return (*this)[n];
  }
  const_reference at(size_type n) const
  {
    THROW_OUT_OF_RANGE_IF(!(n < size()), "deque<T>::at() subscript out of range");
    return (*this)[n]; 
  }

  reference       front()
  {
    MYSTL_DEBUG(!empty());
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
    return *(end() - 1);
  }
  const_reference back() const
  {
    MYSTL_DEBUG(!empty());
    return *(end() - 1);
  }

  // 修改容器相关操作

  // assign

  void     assign(size_type n, const value_type& value)
  { fill_assign(n, value); }

  template <class IIter, typename std::enable_if<
    mystl::is_input_iterator<IIter>::value, int>::type = 0>
  void     assign(IIter first, IIter last)
  { copy_assign(first, last, iterator_category(first)); }

  void     assign(std::initializer_list<value_type> ilist)
  { copy_assign(ilist.begin(), ilist.end(), mystl::forward_iterator_tag{}); }

  // emplace_front / emplace_back / emplace

  template <class ...Args>
  void     emplace_front(Args&& ...args);
  template <class ...Args>
  void     emplace_back(Args&& ...args);
  template <class ...Args>
  iterator emplace(iterator pos, Args&& ...args);

  // push_front / push_back

  void     push_front(const value_type& value);
  void     push_back(const value_type& value);

  void     push_front(value_type&& value) { emplace_front(mystl::move(value)); }
  void     push_back(value_type&& value)  { emplace_back(mystl::move(value)); }

  // pop_back / pop_front

  void     pop_front();
  void     pop_back();

  // insert

  iterator insert(iterator position, const value_type& value);
  iterator insert(iterator position, value_type&& value);
  void     insert(iterator position, size_type n, const value_type& value);
  template <class IIter, typename std::enable_if<
    mystl::is_input_iterator<IIter>::value, int>::type = 0>
  void     insert(iterator position, IIter first, IIter last)
  { insert_dispatch(position, first, last, iterator_category(first)); }

  // erase /clear

  iterator erase(iterator position);
  iterator erase(iterator first, iterator last);
  void     clear();

  // swap

  void     swap(deque& rhs) noexcept;

private:
  // helper functions

  // create node / destroy node
  map_pointer create_map(size_type size);
  void        create_buffer(map_pointer nstart, map_pointer nfinish);
  void        destroy_buffer(map_pointer nstart, map_pointer nfinish);

  // initialize
  void        map_init(size_type nelem);
  void        fill_init(size_type n, const value_type& value);
  template <class IIter>
  void        copy_init(IIter, IIter, input_iterator_tag);
  template <class FIter>
  void        copy_init(FIter, FIter, forward_iterator_tag);

  // assign
  void        fill_assign(size_type n, const value_type& value);
  template <class IIter>
  void        copy_assign(IIter first, IIter last, input_iterator_tag);
  template <class FIter>
  void        copy_assign(FIter first, FIter last, forward_iterator_tag);

  // insert
  template <class... Args>
  iterator    insert_aux(iterator position, Args&& ...args);
  void        fill_insert(iterator position, size_type n, const value_type& x);
  template <class FIter>
  void        copy_insert(iterator, FIter, FIter, size_type);
  template <class IIter>
  void        insert_dispatch(iterator, IIter, IIter, input_iterator_tag);
  template <class FIter>
  void        insert_dispatch(iterator, FIter, FIter, forward_iterator_tag);

  // reallocate
  void        require_capacity(size_type n, bool front);
  void        reallocate_map_at_front(size_type need);
  void        reallocate_map_at_back(size_type need);

};

/*****************************************************************************************/

// 复制赋值运算符
template <class T>
deque<T>& deque<T>::operator=(const deque& rhs)
{
  if (this != &rhs)
  {
    const auto len = size();
    if (len >= rhs.size())
    {
      erase(mystl::copy(rhs.begin_, rhs.end_, begin_), end_);
    }
    else
    {
      iterator mid = rhs.begin() + static_cast<difference_type>(len);
      mystl::copy(rhs.begin_, mid, begin_);
      insert(end_, mid, rhs.end_);
    }
  }
  return *this;
}

// 移动赋值运算符
template <class T>
deque<T>& deque<T>::operator=(deque&& rhs)
{
  clear();
  begin_ = mystl::move(rhs.begin_);
  end_ = mystl::move(rhs.end_);
  map_ = rhs.map_;
  map_size_ = rhs.map_size_;
  rhs.map_ = nullptr;
  rhs.map_size_ = 0;
  return *this;
}

// 重置容器大小
template <class T>
void deque<T>::resize(size_type new_size, const value_type& value)
{
  const auto len = size();
  if (new_size < len)
  {
    erase(begin_ + new_size, end_);
  }
  else
  {
    insert(end_, new_size - len, value);
  }
}

// 减小容器容量
template <class T>
void deque<T>::shrink_to_fit() noexcept
{
  // 至少会留下头部缓冲区
  for (auto cur = map_; cur < begin_.node; ++cur)
  {
    data_allocator::deallocate(*cur, buffer_size);
    *cur = nullptr;
  }
  for (auto cur = end_.node + 1; cur < map_ + map_size_; ++cur)
  {
    data_allocator::deallocate(*cur, buffer_size);
    *cur = nullptr;
  }
}

// 在头部就地构建元素
template <class T>
template <class ...Args>
void deque<T>::emplace_front(Args&& ...args)
{
  if (begin_.cur != begin_.first)
  {
    data_allocator::construct(begin_.cur - 1, mystl::forward<Args>(args)...);
    --begin_.cur;
  }
  else
  {
    require_capacity(1, true);
    try
    {
      --begin_;
      data_allocator::construct(begin_.cur, mystl::forward<Args>(args)...);
    }
    catch (...)
    {
      ++begin_;
      throw;
    }
  }
}

// 在尾部就地构建元素
template <class T>
template <class ...Args>
void deque<T>::emplace_back(Args&& ...args)
{
  if (end_.cur != end_.last - 1)
  {
    data_allocator::construct(end_.cur, mystl::forward<Args>(args)...);
    ++end_.cur;
  }
  else
  {
    require_capacity(1, false);
    data_allocator::construct(end_.cur, mystl::forward<Args>(args)...);
    ++end_;
  }
}

// 在 pos 位置就地构建元素
template <class T>
template <class ...Args>
typename deque<T>::iterator deque<T>::emplace(iterator pos, Args&& ...args)
{
  if (pos.cur == begin_.cur)
  {
    emplace_front(mystl::forward<Args>(args)...);
    return begin_;
  }
  else if (pos.cur == end_.cur)
  {
    emplace_back(mystl::forward<Args>(args)...);
    return end_ - 1;
  }
  return insert_aux(pos, mystl::forward<Args>(args)...);
}

// 在头部插入元素
template <class T>
void deque<T>::push_front(const value_type& value)
{
  if (begin_.cur != begin_.first)
  {
    data_allocator::construct(begin_.cur - 1, value);
    --begin_.cur;
  }
  else
  {
    require_capacity(1, true);
    try
    {
      --begin_;
      data_allocator::construct(begin_.cur, value);
    }
    catch (...)
    {
      ++begin_;
      throw;
    }
  }
}

// 在尾部插入元素
template <class T>
void deque<T>::push_back(const value_type& value)
{
  if (end_.cur != end_.last - 1)
  {
    data_allocator::construct(end_.cur, value);
    ++end_.cur;
  }
  else
  {
    require_capacity(1, false);
    data_allocator::construct(end_.cur, value);
    ++end_;
  }
}

// 弹出头部元素
template <class T>
void deque<T>::pop_front()
{
  MYSTL_DEBUG(!empty());
  if (begin_.cur != begin_.last - 1)
  {
    data_allocator::destroy(begin_.cur);
    ++begin_.cur;
  }
  else
  {
    data_allocator::destroy(begin_.cur);
    ++begin_;
    destroy_buffer(begin_.node - 1, begin_.node - 1);
  }
}

// 弹出尾部元素
template <class T>
void deque<T>::pop_back()
{
  MYSTL_DEBUG(!empty());
  if (end_.cur != end_.first)
  {
    --end_.cur;
    data_allocator::destroy(end_.cur);
  }
  else
  {
    --end_;
    data_allocator::destroy(end_.cur);
    destroy_buffer(end_.node + 1, end_.node + 1);
  }
}

// 在 position 处插入元素
template <class T>
typename deque<T>::iterator
deque<T>::insert(iterator position, const value_type& value)
{
  if (position.cur == begin_.cur)
  {
    push_front(value);
    return begin_;
  }
  else if (position.cur == end_.cur)
  {
    push_back(value);
    auto tmp = end_;
    --tmp;
    return tmp;
  }
  else
  {
    return insert_aux(position, value);
  }
}

template <class T>
typename deque<T>::iterator
deque<T>::insert(iterator position, value_type&& value)
{
  if (position.cur == begin_.cur)
  {
    emplace_front(mystl::move(value));
    return begin_;
  }
  else if (position.cur == end_.cur)
  {
    emplace_back(mystl::move(value));
    auto tmp = end_;
    --tmp;
    return tmp;
  }
  else
  {
    return insert_aux(position, mystl::move(value));
  }
}

// 在 position 位置插入 n 个元素
template <class T>
void deque<T>::insert(iterator position, size_type n, const value_type& value)
{
  if (position.cur == begin_.cur)
  {
    require_capacity(n, true);
    auto new_begin = begin_ - n;
    mystl::uninitialized_fill_n(new_begin, n, value);
    begin_ = new_begin;
  }
  else if (position.cur == end_.cur)
  {
    require_capacity(n, false);
    auto new_end = end_ + n;
    mystl::uninitialized_fill_n(end_, n, value);
    end_ = new_end;
  }
  else
  {
    fill_insert(position, n, value);
  }
}

// 删除 position 处的元素
template <class T>
typename deque<T>::iterator
deque<T>::erase(iterator position)
{
  auto next = position;
  ++next;
  const size_type elems_before = position - begin_;
  if (elems_before < (size() / 2))
  {
    mystl::copy_backward(begin_, position, next);
    pop_front();
  }
  else
  {
    mystl::copy(next, end_, position);
    pop_back();
  }
  return begin_ + elems_before;
}

// 删除[first, last)上的元素
template <class T>
typename deque<T>::iterator
deque<T>::erase(iterator first, iterator last)
{
  if (first == begin_ && last == end_)
  {
    clear();
    return end_;
  }
  else
  {
    const size_type len = last - first;
    const size_type elems_before = first - begin_;
    if (elems_before < ((size() - len) / 2))
    {
      mystl::copy_backward(begin_, first, last);
      auto new_begin = begin_ + len;
      data_allocator::destroy(begin_.cur, new_begin.cur);
      begin_ = new_begin;
    }
    else
    {
      mystl::copy(last, end_, first);
      auto new_end = end_ - len;
      data_allocator::destroy(new_end.cur, end_.cur);
      end_ = new_end;
    }
    return begin_ + elems_before;
  }
}

// 清空 deque
template <class T>
void deque<T>::clear()
{
  // clear 会保留头部的缓冲区
  for (map_pointer cur = begin_.node + 1; cur < end_.node; ++cur)
  {
    data_allocator::destroy(*cur, *cur + buffer_size);
  }
  if (begin_.node != end_.node)
  { // 有两个以上的缓冲区
    mystl::destroy(begin_.cur, begin_.last);
    mystl::destroy(end_.first, end_.cur);
  }
  else
  {
    mystl::destroy(begin_.cur, end_.cur);
  }
  shrink_to_fit();
  end_ = begin_;
}

// 交换两个 deque
template <class T>
void deque<T>::swap(deque& rhs) noexcept
{
  if (this != &rhs)
  {
    mystl::swap(begin_, rhs.begin_);
    mystl::swap(end_, rhs.end_);
    mystl::swap(map_, rhs.map_);
    mystl::swap(map_size_, rhs.map_size_);
  }
}

/*****************************************************************************************/
// helper function

template <class T>
typename deque<T>::map_pointer
deque<T>::create_map(size_type size)
{
  map_pointer mp = nullptr;
  mp = map_allocator::allocate(size);
  for (size_type i = 0; i < size; ++i)
    *(mp + i) = nullptr;
  return mp;
}

// create_buffer 函数
template <class T>
void deque<T>::
create_buffer(map_pointer nstart, map_pointer nfinish)
{
  map_pointer cur;
  try
  {
    for (cur = nstart; cur <= nfinish; ++cur)
    {
      *cur = data_allocator::allocate(buffer_size);
    }
  }
  catch (...)
  {
    while (cur != nstart)
    {
      --cur;
      data_allocator::deallocate(*cur, buffer_size);
      *cur = nullptr;
    }
    throw;
  }
}

// destroy_buffer 函数
template <class T>
void deque<T>::
destroy_buffer(map_pointer nstart, map_pointer nfinish)
{
  for (map_pointer n = nstart; n <= nfinish; ++n)
  {
    data_allocator::deallocate(*n, buffer_size);
    *n = nullptr;
  }
}

// map_init 函数
template <class T>
void deque<T>::
map_init(size_type nElem)
{
  const size_type nNode = nElem / buffer_size + 1;  // 需要分配的缓冲区个数
  map_size_ = mystl::max(static_cast<size_type>(DEQUE_MAP_INIT_SIZE), nNode + 2);
  try
  {
    map_ = create_map(map_size_);
  }
  catch (...)
  {
    map_ = nullptr;
    map_size_ = 0;
    throw;
  }

  // 让 nstart 和 nfinish 都指向 map_ 最中央的区域，方便向头尾扩充
  map_pointer nstart = map_ + (map_size_ - nNode) / 2;
  map_pointer nfinish = nstart + nNode - 1;
  try
  {
    create_buffer(nstart, nfinish);
  }
  catch (...)
  {
    map_allocator::deallocate(map_, map_size_);
    map_ = nullptr;
    map_size_ = 0;
    throw;
  }
  begin_.set_node(nstart);
  end_.set_node(nfinish);
  begin_.cur = begin_.first;
  end_.cur = end_.first + (nElem % buffer_size);
}

// fill_init 函数
template <class T>
void deque<T>::
fill_init(size_type n, const value_type& value)
{
  map_init(n);
  if (n != 0)
  {
    for (auto cur = begin_.node; cur < end_.node; ++cur)
    {
      mystl::uninitialized_fill(*cur, *cur + buffer_size, value);
    }
    mystl::uninitialized_fill(end_.first, end_.cur, value);
  }
}

// copy_init 函数
template <class T>
template <class IIter>
void deque<T>::
copy_init(IIter first, IIter last, input_iterator_tag)
{
  const size_type n = mystl::distance(first, last);
  map_init(n);
  for (; first != last; ++first)
    emplace_back(*first);
}

template <class T>
template <class FIter>
void deque<T>::
copy_init(FIter first, FIter last, forward_iterator_tag)
{
  const size_type n = mystl::distance(first, last);
  map_init(n);
  for (auto cur = begin_.node; cur < end_.node; ++cur)
  {
    auto next = first;
    mystl::advance(next, buffer_size);
    mystl::uninitialized_copy(first, next, *cur);
    first = next;
  }
  mystl::uninitialized_copy(first, last, end_.first);
}

// fill_assign 函数
template <class T>
void deque<T>::
fill_assign(size_type n, const value_type& value)
{
  if (n > size())
  {
    mystl::fill(begin(), end(), value);
    insert(end(), n - size(), value);
  }
  else
  {
    erase(begin() + n, end());
    mystl::fill(begin(), end(), value);
  }
}

// copy_assign 函数
template <class T>
template <class IIter>
void deque<T>::
copy_assign(IIter first, IIter last, input_iterator_tag)
{
  auto first1 = begin();
  auto last1 = end();
  for (; first != last && first1 != last1; ++first, ++first1)
  {
    *first1 = *first;
  }
  if (first1 != last1)
  {
    erase(first1, last1);
  }
  else
  {
    insert_dispatch(end_, first, last, input_iterator_tag{});
  }
}

template <class T>
template <class FIter>
void deque<T>::
copy_assign(FIter first, FIter last, forward_iterator_tag)
{  
  const size_type len1 = size();
  const size_type len2 = mystl::distance(first, last);
  if (len1 < len2)
  {
    auto next = first;
    mystl::advance(next, len1);
    mystl::copy(first, next, begin_);
    insert_dispatch(end_, next, last, forward_iterator_tag{});
  }
  else
  {
    erase(mystl::copy(first, last, begin_), end_);
  }
}

// insert_aux 函数
template <class T>
template <class... Args>
typename deque<T>::iterator
deque<T>::
insert_aux(iterator position, Args&& ...args)
{
  const size_type elems_before = position - begin_;
  value_type value_copy = value_type(mystl::forward<Args>(args)...);
  if (elems_before < (size() / 2))
  { // 在前半段插入
    emplace_front(front());
    auto front1 = begin_;
    ++front1;
    auto front2 = front1;
    ++front2;
    position = begin_ + elems_before;
    auto pos = position;
    ++pos;
    mystl::copy(front2, pos, front1);
  }
  else
  { // 在后半段插入
    emplace_back(back());
    auto back1 = end_;
    --back1;
    auto back2 = back1;
    --back2;
    position = begin_ + elems_before;
    mystl::copy_backward(position, back2, back1);
  }
  *position = mystl::move(value_copy);
  return position;
}

// fill_insert 函数
template <class T>
void deque<T>::
fill_insert(iterator position, size_type n, const value_type& value)
{
  const size_type elems_before = position - begin_;
  const size_type len = size();
  auto value_copy = value;
  if (elems_before < (len / 2))
  {
    require_capacity(n, true);
    // 原来的迭代器可能会失效
    auto old_begin = begin_;
    auto new_begin = begin_ - n;
    position = begin_ + elems_before;
    try
    {
      if (elems_before >= n)
      {
        auto begin_n = begin_ + n;
        mystl::uninitialized_copy(begin_, begin_n, new_begin);
        begin_ = new_begin;
        mystl::copy(begin_n, position, old_begin);
        mystl::fill(position - n, position, value_copy);
      }
      else
      {
        mystl::uninitialized_fill(
          mystl::uninitialized_copy(begin_, position, new_begin), begin_, value_copy);
        begin_ = new_begin;
        mystl::fill(old_begin, position, value_copy);
      }
    }
    catch (...)
    {
      if (new_begin.node != begin_.node)
        destroy_buffer(new_begin.node, begin_.node - 1);
      throw;
    }
  }
  else
  {
    require_capacity(n, false);
    // 原来的迭代器可能会失效
    auto old_end = end_;
    auto new_end = end_ + n;
    const size_type elems_after = len - elems_before;
    position = end_ - elems_after;
    try
    {
      if (elems_after > n)
      {
        auto end_n = end_ - n;
        mystl::uninitialized_copy(end_n, end_, end_);
        end_ = new_end;
        mystl::copy_backward(position, end_n, old_end);
        mystl::fill(position, position + n, value_copy);
      }
      else
      {
        mystl::uninitialized_fill(end_, position + n, value_copy);
        mystl::uninitialized_copy(position, end_, position + n);
        end_ = new_end;
        mystl::fill(position, old_end, value_copy);
      }
    }
    catch (...)
    {
      if(new_end.node != end_.node)
        destroy_buffer(end_.node + 1, new_end.node);
      throw;
    }
  }
}

// copy_insert
template <class T>
template <class FIter>
void deque<T>::
copy_insert(iterator position, FIter first, FIter last, size_type n)
{
  const size_type elems_before = position - begin_;
  auto len = size();
  if (elems_before < (len / 2))
  {
    require_capacity(n, true);
    // 原来的迭代器可能会失效
    auto old_begin = begin_;
    auto new_begin = begin_ - n;
    position = begin_ + elems_before;
    try
    {
      if (elems_before >= n)
      {
        auto begin_n = begin_ + n;
        mystl::uninitialized_copy(begin_, begin_n, new_begin);
        begin_ = new_begin;
        mystl::copy(begin_n, position, old_begin);
        mystl::copy(first, last, position - n);
      }
      else
      {
        auto mid = first;
        mystl::advance(mid, n - elems_before);
        mystl::uninitialized_copy(first, mid,
                                  mystl::uninitialized_copy(begin_, position, new_begin));
        begin_ = new_begin;
        mystl::copy(mid, last, old_begin);
      }
    }
    catch (...)
    {
      if(new_begin.node != begin_.node)
        destroy_buffer(new_begin.node, begin_.node - 1);
      throw;
    }
  }
  else
  {
    require_capacity(n, false);
    // 原来的迭代器可能会失效
    auto old_end = end_;
    auto new_end = end_ + n;
    const auto elems_after = len - elems_before;
    position = end_ - elems_after;
    try
    {
      if (elems_after > n)
      {
        auto end_n = end_ - n;
        mystl::uninitialized_copy(end_n, end_, end_);
        end_ = new_end;
        mystl::copy_backward(position, end_n, old_end);
        mystl::copy(first, last, position);
      }
      else
      {
        auto mid = first;
        mystl::advance(mid, elems_after);
        mystl::uninitialized_copy(position, end_,
                                  mystl::uninitialized_copy(mid, last, end_));
        end_ = new_end;
        mystl::copy(first, mid, position);
      }
    }
    catch (...)
    {
      if(new_end.node != end_.node)
        destroy_buffer(end_.node + 1, new_end.node);
      throw;
    }
  }
}

// insert_dispatch 函数
template <class T>
template <class IIter>
void deque<T>::
insert_dispatch(iterator position, IIter first, IIter last, input_iterator_tag)
{
  if (last <= first)  return;
  const size_type n = mystl::distance(first, last);
  const size_type elems_before = position - begin_;
  if (elems_before < (size() / 2))
  {
    require_capacity(n, true);
  }
  else
  {
    require_capacity(n, false);
  }
  position = begin_ + elems_before;
  auto cur = --last;
  for (size_type i = 0; i < n; ++i, --cur)
  {
    insert(position, *cur);
  }
}

template <class T>
template <class FIter>
void deque<T>::
insert_dispatch(iterator position, FIter first, FIter last, forward_iterator_tag)
{
  if (last <= first)  return;
  const size_type n = mystl::distance(first, last);
  if (position.cur == begin_.cur)
  {
    require_capacity(n, true);
    auto new_begin = begin_ - n;
    try
    {
      mystl::uninitialized_copy(first, last, new_begin);
      begin_ = new_begin;
    }
    catch (...)
    {
      if(new_begin.node != begin_.node)
        destroy_buffer(new_begin.node, begin_.node - 1);
      throw;
    }
  }
  else if (position.cur == end_.cur)
  {
    require_capacity(n, false);
    auto new_end = end_ + n;
    try
    {
      mystl::uninitialized_copy(first, last, end_);
      end_ = new_end;
    }
    catch (...)
    {
      if(new_end.node != end_.node)
        destroy_buffer(end_.node + 1, new_end.node);
      throw;
    }
  }
  else
  {
    copy_insert(position, first, last, n);
  }
}

// require_capacity 函数
template <class T>
void deque<T>::require_capacity(size_type n, bool front)
{
  if (front && (static_cast<size_type>(begin_.cur - begin_.first) < n))
  {
    const size_type need_buffer = (n - (begin_.cur - begin_.first)) / buffer_size + 1;
    if (need_buffer > static_cast<size_type>(begin_.node - map_))
    {
      reallocate_map_at_front(need_buffer);
      return;
    }
    create_buffer(begin_.node - need_buffer, begin_.node - 1);
  }
  else if (!front && (static_cast<size_type>(end_.last - end_.cur - 1) < n))
  {
    const size_type need_buffer = (n - (end_.last - end_.cur - 1)) / buffer_size + 1;
    if (need_buffer > static_cast<size_type>((map_ + map_size_) - end_.node - 1))
    {
      reallocate_map_at_back(need_buffer);
      return;
    }
    create_buffer(end_.node + 1, end_.node + need_buffer);
  }
}

// reallocate_map_at_front 函数
template <class T>
void deque<T>::reallocate_map_at_front(size_type need_buffer)
{
  const size_type new_map_size = mystl::max(map_size_ << 1,
                                            map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
  map_pointer new_map = create_map(new_map_size);
  const size_type old_buffer = end_.node - begin_.node + 1;
  const size_type new_buffer = old_buffer + need_buffer;

  // 另新的 map 中的指针指向原来的 buffer，并开辟新的 buffer
  auto begin = new_map + (new_map_size - new_buffer) / 2;
  auto mid = begin + need_buffer;
  auto end = mid + old_buffer;
  create_buffer(begin, mid - 1);
  for (auto begin1 = mid, begin2 = begin_.node; begin1 != end; ++begin1, ++begin2)
    *begin1 = *begin2;

  // 更新数据
  map_allocator::deallocate(map_, map_size_);
  map_ = new_map;
  map_size_ = new_map_size;
  begin_ = iterator(*mid + (begin_.cur - begin_.first), mid);
  end_ = iterator(*(end - 1) + (end_.cur - end_.first), end - 1);
}

// reallocate_map_at_back 函数
template <class T>
void deque<T>::reallocate_map_at_back(size_type need_buffer)
{
  const size_type new_map_size = mystl::max(map_size_ << 1,
                                            map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
  map_pointer new_map = create_map(new_map_size);
  const size_type old_buffer = end_.node - begin_.node + 1;
  const size_type new_buffer = old_buffer + need_buffer;

  // 另新的 map 中的指针指向原来的 buffer，并开辟新的 buffer
  auto begin = new_map + ((new_map_size - new_buffer) / 2);
  auto mid = begin + old_buffer;
  auto end = mid + need_buffer;
  for (auto begin1 = begin, begin2 = begin_.node; begin1 != mid; ++begin1, ++begin2)
    *begin1 = *begin2;
  create_buffer(mid, end - 1);

  // 更新数据
  map_allocator::deallocate(map_, map_size_);
  map_ = new_map;
  map_size_ = new_map_size;
  begin_ = iterator(*begin + (begin_.cur - begin_.first), begin);
  end_ = iterator(*(mid - 1) + (end_.cur - end_.first), mid - 1);
}

// 重载比较操作符
template <class T>
bool operator==(const deque<T>& lhs, const deque<T>& rhs)
{
  return lhs.size() == rhs.size() && 
    mystl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class T>
bool operator<(const deque<T>& lhs, const deque<T>& rhs)
{
  return mystl::lexicographical_compare(
    lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class T>
bool operator!=(const deque<T>& lhs, const deque<T>& rhs)
{
  return !(lhs == rhs);
}

template <class T>
bool operator>(const deque<T>& lhs, const deque<T>& rhs)
{
  return rhs < lhs;
}

template <class T>
bool operator<=(const deque<T>& lhs, const deque<T>& rhs)
{
  return !(rhs < lhs);
}

template <class T>
bool operator>=(const deque<T>& lhs, const deque<T>& rhs)
{
  return !(lhs < rhs);
}

// 重载 mystl 的 swap
template <class T>
void swap(deque<T>& lhs, deque<T>& rhs)
{
  lhs.swap(rhs);
}

} // namespace mystl
#endif // !MYTINYSTL_DEQUE_H_

