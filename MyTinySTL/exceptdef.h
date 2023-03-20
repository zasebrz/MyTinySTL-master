#ifndef MYTINYSTL_EXCEPTDEF_H_
#define MYTINYSTL_EXCEPTDEF_H_

#include <stdexcept>

#include <cassert>

namespace mystl
{
	//#define后面的"\"是续行符,表示下面一行是紧接着当前行的,一般用于将十分长的代码语句分几段写(语句本身要求必须是一行)。
#define MYSTL_DEBUG(expr) \
  assert(expr)  

#define THROW_LENGTH_ERROR_IF(expr, what) \
  if ((expr)) throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr, what) \
  if ((expr)) throw std::out_of_range(what)//expr用了两个括号是因为define的简单替换可能会因为运算符的优先级而出现错误
									       //打了括号以后保证先计算expr

#define THROW_RUNTIME_ERROR_IF(expr, what) \
  if ((expr)) throw std::runtime_error(what)

} // namepsace mystl

#endif // !MYTINYSTL_EXCEPTDEF_H_

