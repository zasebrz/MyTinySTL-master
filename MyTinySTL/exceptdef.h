#ifndef MYTINYSTL_EXCEPTDEF_H_
#define MYTINYSTL_EXCEPTDEF_H_

#include <stdexcept>

#include <cassert>

namespace mystl
{
	//#define�����"\"�����з�,��ʾ����һ���ǽ����ŵ�ǰ�е�,һ�����ڽ�ʮ�ֳ��Ĵ������ּ���д(��䱾��Ҫ�������һ��)��
#define MYSTL_DEBUG(expr) \
  assert(expr)  

#define THROW_LENGTH_ERROR_IF(expr, what) \
  if ((expr)) throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr, what) \
  if ((expr)) throw std::out_of_range(what)//expr����������������Ϊdefine�ļ��滻���ܻ���Ϊ����������ȼ������ִ���
									       //���������Ժ�֤�ȼ���expr

#define THROW_RUNTIME_ERROR_IF(expr, what) \
  if ((expr)) throw std::runtime_error(what)

} // namepsace mystl

#endif // !MYTINYSTL_EXCEPTDEF_H_

