#ifndef MYTINYSTL_ALGOBASE_H_
#define MYTINYSTL_ALGOBASE_H_

// 这个头文件包含了 mystl 的基本算法

#include <cstring>

#include "iterator.h"
#include "util.h"

namespace mystl
{

#ifdef max//如果之前定义了max，输出#undefing marco max，然后取消定义max，此头文件会重新定义
#pragma message("#undefing marco max")//是在编译期间，将一个文字串（messagestring）发送到标准输出窗口，没有实际作用，对代码不影响
#undef max
#endif // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif // min

/*****************************************************************************************/
// max
// 取二者中的较大值，语义相等时保证返回第一个参数
/*****************************************************************************************/
template <class T>
const T& max(const T& lhs, const T& rhs)
{
  return lhs < rhs ? rhs : lhs;
}

// 重载版本使用函数对象 comp 代替比较操作
template <class T, class Compare>
const T& max(const T& lhs, const T& rhs, Compare comp)
{
  return comp(lhs, rhs) ? rhs : lhs;
}

/*****************************************************************************************/
// min 
// 取二者中的较小值，语义相等时保证返回第一个参数
/*****************************************************************************************/
template <class T>
const T& min(const T& lhs, const T& rhs)
{
  return rhs < lhs ? rhs : lhs;
}

// 重载版本使用函数对象 comp 代替比较操作
template <class T, class Compare>
const T& min(const T& lhs, const T& rhs, Compare comp)
{
  return comp(rhs, lhs) ? rhs : lhs;
}

/*****************************************************************************************/
// iter_swap
// 将两个迭代器所指对象对调
/*****************************************************************************************/
template <class FIter1, class FIter2>
void iter_swap(FIter1 lhs, FIter2 rhs)
{
  mystl::swap(*lhs, *rhs);
}

/*****************************************************************************************/
// copy
// 把 [first, last)区间内的元素拷贝到 [result, result + (last - first))内
/*****************************************************************************************/
// input_iterator_tag 版本
template <class InputIter, class OutputIter>
OutputIter 
unchecked_copy_cat(InputIter first, InputIter last, OutputIter result, 
                   mystl::input_iterator_tag)
{
  for (; first != last; ++first, ++result)
  {
    *result = *first;
  }
  return result;
}

// ramdom_access_iterator_tag 版本
template <class RandomIter, class OutputIter>
OutputIter 
unchecked_copy_cat(RandomIter first, RandomIter last, OutputIter result,
                   mystl::random_access_iterator_tag)
{
  for (auto n = last - first; n > 0; --n, ++first, ++result)
  {
    *result = *first;
  }
  return result;
}

template <class InputIter, class OutputIter>
OutputIter 
unchecked_copy(InputIter first, InputIter last, OutputIter result)
{
  return unchecked_copy_cat(first, last, result, iterator_category(first));
}

// 为 trivially_copy_assignable 类型提供特化版本
template <class Tp, class Up>
typename std::enable_if<
  std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
  std::is_trivially_copy_assignable<Up>::value,
  Up*>::type
//分解来看，首先这是一个函数的返回值，返回一个类型，std::enable_if<>::type
//std::enable_if<>一般是由两部分组成，第一个参数是一个判定式，当判定式为真时，
//std::enable_if<>::type的这个type类型成员存在，且类型和第二个参数相同，如果没有第二个参数，则默认值为void。
//如果判定式为假，type这个类型成员就是未定义的，更不存在值是什么。
//再来看，那一长串就是enable_if的第一个参数，是一个判定式，有两个bool值相与得到最终的值，第一个是is_same<>
//他用来判断两个参数是否是相同类型，这里is_same<>的第一个参数是去掉const属性的Tp，输入迭代器指向元素的类型
//第二个参数是Up，输出迭代器指向元素的类型，如果两个不相同，那么memmove肯定用不了（把int的内存映像拷贝到double内存去的行为未定义）
//如果两个相同，则进行判别式的第二个判断，is_trivially_copy_assignable用来指出Up是否是CV限定的类型
//如果是的话，那么is_trivially_copy_assignable<Up>::value就是true，否则就是false，我们知道，CV限定的类型没有赋值拷贝运算符
//因此只能通过内存映像拷贝的方式进行复制，也就是memmove函数。当这两个判断都是true的时候，判别式为true，std::enable_if<>::type
//等于它的第二个参数，也就是Up*，这里返回的是指针，因此内存拷贝只能返回指针
unchecked_copy(Tp* first, Tp* last, Up* result)
{//我们要把[first，last)区间内的元素复制到result内存地址上去
  const auto n = static_cast<size_t>(last - first);//计算一下指针相差的距离，也就是相差几个指针
  if (n != 0)//memmove：能够对本身进行覆盖拷贝的函数，原型是void *memmove(void *dest, const void *source, size_t count)
            //count为要拷贝字节的个数，n是指针，乘以Up或Tp的大小就是拷贝字节数
    std::memmove(result, first, n * sizeof(Up));
  return result + n;
}

template <class InputIter, class OutputIter>
OutputIter copy(InputIter first, InputIter last, OutputIter result)
{
  return unchecked_copy(first, last, result);
  //通过判断 迭代器指向类型（OutputIter） 有没有定义拷贝赋值运算符（通过const/volatile，CV修饰符限定的对象都不能有拷贝赋值运算符）
  //如果没有定义直接通过memmove拷贝效率最高，定义了的话就调用unchecked_copy_cat，通过=号逐个拷贝
}

/*****************************************************************************************/
// copy_backward
// 将 [first, last)区间内的元素拷贝到 [result - (last - first), result)内
/*****************************************************************************************/
// unchecked_copy_backward_cat 的 bidirectional_iterator_tag 版本
template <class BidirectionalIter1, class BidirectionalIter2>
BidirectionalIter2 
unchecked_copy_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
                            BidirectionalIter2 result, mystl::bidirectional_iterator_tag)
{
  while (first != last)
    *--result = *--last;
  return result;
}

// unchecked_copy_backward_cat 的 random_access_iterator_tag 版本
template <class BidirectionalIter1, class BidirectionalIter2>
BidirectionalIter2 
unchecked_copy_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
                            BidirectionalIter2 result, mystl::random_access_iterator_tag)
{
  for (auto n = last - first; n > 0; --n)
    *--result = *--last;
  return result;
}

template <class BidirectionalIter1, class BidirectionalIter2>
BidirectionalIter2 
unchecked_copy_backward(BidirectionalIter1 first, BidirectionalIter1 last,
                        BidirectionalIter2 result)
{
  return unchecked_copy_backward_cat(first, last, result,
                                     iterator_category(first));
}

// 为 trivially_copy_assignable 类型提供特化版本
template <class Tp, class Up>
typename std::enable_if<
  std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
  std::is_trivially_copy_assignable<Up>::value,
  Up*>::type
unchecked_copy_backward(Tp* first, Tp* last, Up* result)
{
  const auto n = static_cast<size_t>(last - first);
  if (n != 0)
  {
    result -= n;//先移动再拷贝
    std::memmove(result, first, n * sizeof(Up));
  }
  return result;
}

template <class BidirectionalIter1, class BidirectionalIter2>
BidirectionalIter2 
copy_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
{
  return unchecked_copy_backward(first, last, result);
}

/*****************************************************************************************/
// copy_if
// 把[first, last)内满足一元操作 unary_pred 的元素拷贝到以 result 为起始的位置上
/*****************************************************************************************/
template <class InputIter, class OutputIter, class UnaryPredicate>
OutputIter 
copy_if(InputIter first, InputIter last, OutputIter result, UnaryPredicate unary_pred)
{
  for (; first != last; ++first)
  {
    if (unary_pred(*first))
      *result++ = *first;
  }
  return result;
}

/*****************************************************************************************/
// copy_n
// 把 [first, first + n)区间上的元素拷贝到 [result, result + n)上
// 返回一个 pair 分别指向拷贝结束的尾部
/*****************************************************************************************/
template <class InputIter, class Size, class OutputIter>
mystl::pair<InputIter, OutputIter>
unchecked_copy_n(InputIter first, Size n, OutputIter result, mystl::input_iterator_tag)
{
  for (; n > 0; --n, ++first, ++result)
  {
    *result = *first;
  }
  return mystl::pair<InputIter, OutputIter>(first, result);
}

template <class RandomIter, class Size, class OutputIter>
mystl::pair<RandomIter, OutputIter>
unchecked_copy_n(RandomIter first, Size n, OutputIter result, 
                 mystl::random_access_iterator_tag)
{
  auto last = first + n;
  return mystl::pair<RandomIter, OutputIter>(last, mystl::copy(first, last, result));
}

template <class InputIter, class Size, class OutputIter>
mystl::pair<InputIter, OutputIter> 
copy_n(InputIter first, Size n, OutputIter result)
{
  return unchecked_copy_n(first, n, result, iterator_category(first));
}

/*****************************************************************************************/
// move
// 把 [first, last)区间内的元素移动到 [result, result + (last - first))内
/*****************************************************************************************/
// input_iterator_tag 版本
template <class InputIter, class OutputIter>
OutputIter 
unchecked_move_cat(InputIter first, InputIter last, OutputIter result,
                   mystl::input_iterator_tag)
{
  for (; first != last; ++first, ++result)
  {
    *result = mystl::move(*first);
  }
  return result;
}

// ramdom_access_iterator_tag 版本
template <class RandomIter, class OutputIter>
OutputIter 
unchecked_move_cat(RandomIter first, RandomIter last, OutputIter result,
                   mystl::random_access_iterator_tag)
{
  for (auto n = last - first; n > 0; --n, ++first, ++result)
  {
    *result = mystl::move(*first);//如果first不是基础数据类型（int，double这些）而是自定义的类，那么这里将调用OutputIter类的
                                  //移动赋值运算符，而不是拷贝赋值，和上面copy函数是不一样的
  }
  return result;
}

template <class InputIter, class OutputIter>
OutputIter 
unchecked_move(InputIter first, InputIter last, OutputIter result)
{
  return unchecked_move_cat(first, last, result, iterator_category(first));
}

// 为 trivially_copy_assignable 类型提供特化版本
template <class Tp, class Up>
typename std::enable_if<
  std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
  std::is_trivially_move_assignable<Up>::value,
  Up*>::type
unchecked_move(Tp* first, Tp* last, Up* result)
{
  const size_t n = static_cast<size_t>(last - first);
  if (n != 0)
    std::memmove(result, first, n * sizeof(Up));//这里用的是拷贝？
  return result + n;
}

template <class InputIter, class OutputIter>
OutputIter move(InputIter first, InputIter last, OutputIter result)
{
  return unchecked_move(first, last, result);
}

/*****************************************************************************************/
// move_backward
// 将 [first, last)区间内的元素移动到 [result - (last - first), result)内
/*****************************************************************************************/
// bidirectional_iterator_tag 版本
template <class BidirectionalIter1, class BidirectionalIter2>
BidirectionalIter2
unchecked_move_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
                            BidirectionalIter2 result, mystl::bidirectional_iterator_tag)
{
  while (first != last)
    *--result = mystl::move(*--last);
  return result;
}

// random_access_iterator_tag 版本
template <class RandomIter1, class RandomIter2>
RandomIter2
unchecked_move_backward_cat(RandomIter1 first, RandomIter1 last,
                            RandomIter2 result, mystl::random_access_iterator_tag)
{
  for (auto n = last - first; n > 0; --n)
    *--result = mystl::move(*--last);
  return result;
}

template <class BidirectionalIter1, class BidirectionalIter2>
BidirectionalIter2
unchecked_move_backward(BidirectionalIter1 first, BidirectionalIter1 last, 
                        BidirectionalIter2 result)
{
  return unchecked_move_backward_cat(first, last, result,
                                     iterator_category(first));
}

// 为 trivially_copy_assignable 类型提供特化版本
template <class Tp, class Up>
typename std::enable_if<
  std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
  std::is_trivially_move_assignable<Up>::value,
  Up*>::type
unchecked_move_backward(Tp* first, Tp* last, Up* result)
{
  const size_t n = static_cast<size_t>(last - first);
  if (n != 0)
  {
    result -= n;
    std::memmove(result, first, n * sizeof(Up));
  }
  return result;
}

template <class BidirectionalIter1, class BidirectionalIter2>
BidirectionalIter2
move_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
{
  return unchecked_move_backward(first, last, result);
}

/*****************************************************************************************/
// equal
// 比较第一序列在 [first, last)区间上的元素值是否和第二序列相等
/*****************************************************************************************/
template <class InputIter1, class InputIter2>
bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2)
{
  for (; first1 != last1; ++first1, ++first2)
  {
    if (*first1 != *first2)  
      return false;
  }
  return true;
}

// 重载版本使用函数对象 comp 代替比较操作
template <class InputIter1, class InputIter2, class Compared>
bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compared comp)
{
  for (; first1 != last1; ++first1, ++first2)
  {
    if (!comp(*first1, *first2))  
      return false;
  }
  return true;
}

/*****************************************************************************************/
// fill_n
// 从 first 位置开始填充 n 个值
/*****************************************************************************************/
template <class OutputIter, class Size, class T>
OutputIter unchecked_fill_n(OutputIter first, Size n, const T& value)
{
  for (; n > 0; --n, ++first)
  {
    *first = value;
  }
  return first;
}

// 为 one-byte 类型提供特化版本
template <class Tp, class Size, class Up>
typename std::enable_if<
  std::is_integral<Tp>::value && sizeof(Tp) == 1 &&//保证Tp是整形，且大小为1字节，只能是bool或char
  !std::is_same<Tp, bool>::value &&//不能是bool，只能是char
  std::is_integral<Up>::value && sizeof(Up) == 1,
  Tp*>::type
unchecked_fill_n(Tp* first, Size n, Up value)
{
  if (n > 0)
  {
    std::memset(first, (unsigned char)value, (size_t)(n));//内存操作更高效，连续设置n个value值，直接对内存设置
  }
  return first + n;
}

template <class OutputIter, class Size, class T>
OutputIter fill_n(OutputIter first, Size n, const T& value)
{
  return unchecked_fill_n(first, n, value);
}

/*****************************************************************************************/
// fill
// 为 [first, last)区间内的所有元素填充新值
/*****************************************************************************************/
template <class ForwardIter, class T>
void fill_cat(ForwardIter first, ForwardIter last, const T& value,
              mystl::forward_iterator_tag)
{
  for (; first != last; ++first)
  {
    *first = value;
  }
}

template <class RandomIter, class T>
void fill_cat(RandomIter first, RandomIter last, const T& value,
              mystl::random_access_iterator_tag)
{
  fill_n(first, last - first, value);
}

template <class ForwardIter, class T>
void fill(ForwardIter first, ForwardIter last, const T& value)
{
  fill_cat(first, last, value, iterator_category(first));
}

/*****************************************************************************************/
// lexicographical_compare
// 以字典序排列对两个序列进行比较，当在某个位置发现第一组不相等元素时，有下列几种情况：
// (1)如果第一序列的元素较小，返回 true ，否则返回 false
// (2)如果到达 last1 而尚未到达 last2 返回 true
// (3)如果到达 last2 而尚未到达 last1 返回 false
// (4)如果同时到达 last1 和 last2 返回 false
/*****************************************************************************************/
template <class InputIter1, class InputIter2>
bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
                             InputIter2 first2, InputIter2 last2)
{
  for (; first1 != last1 && first2 != last2; ++first1, ++first2)
  {
    if (*first1 < *first2)
      return true;
    if (*first2 < *first1)
      return false;
  }
  return first1 == last1 && first2 != last2;
}

// 重载版本使用函数对象 comp 代替比较操作
template <class InputIter1, class InputIter2, class Compred>
bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
                             InputIter2 first2, InputIter2 last2, Compred comp)
{
  for (; first1 != last1 && first2 != last2; ++first1, ++first2)
  {
    if (comp(*first1, *first2))
      return true;
    if (comp(*first2, *first1))
      return false;
  }
  return first1 == last1 && first2 != last2;
}

// 针对 const unsigned char* 的特化版本
bool lexicographical_compare(const unsigned char* first1,
                             const unsigned char* last1,
                             const unsigned char* first2,
                             const unsigned char* last2)
{
  const auto len1 = last1 - first1;
  const auto len2 = last2 - first2;
  // 先比较相同长度的部分
  const auto result = std::memcmp(first1, first2, mystl::min(len1, len2));
  // 若相等，长度较长的比较大
  return result != 0 ? result < 0 : len1 < len2;
}

/*****************************************************************************************/
// mismatch
// 平行比较两个序列，找到第一处失配的元素，返回一对迭代器，分别指向两个序列中失配的元素
/*****************************************************************************************/
template <class InputIter1, class InputIter2>
mystl::pair<InputIter1, InputIter2> 
mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2)
{
  while (first1 != last1 && *first1 == *first2)
  {
    ++first1;
    ++first2;
  }
  return mystl::pair<InputIter1, InputIter2>(first1, first2);
}

// 重载版本使用函数对象 comp 代替比较操作
template <class InputIter1, class InputIter2, class Compred>
mystl::pair<InputIter1, InputIter2> 
mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compred comp)
{
  while (first1 != last1 && comp(*first1, *first2))
  {
    ++first1;
    ++first2;
  }
  return mystl::pair<InputIter1, InputIter2>(first1, first2);
}

} // namespace mystl
#endif // !MYTINYSTL_ALGOBASE_H_

