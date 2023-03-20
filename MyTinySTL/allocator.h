#ifndef MYTINYSTL_ALLOCATOR_H_
#define MYTINYSTL_ALLOCATOR_H_

// 这个头文件包含一个模板类 allocator，用于管理内存的分配、释放，对象的构造、析构

#include "construct.h"
#include "util.h"

namespace mystl
{

// 模板类：allocator
// 模板函数代表数据类型
template <class T>
class allocator
{
public:
  typedef T            value_type;
  typedef T*           pointer;
  typedef const T*     const_pointer;
  typedef T&           reference;
  typedef const T&     const_reference;
  typedef size_t       size_type;
  typedef ptrdiff_t    difference_type;

public:
  static T*   allocate();
  static T*   allocate(size_type n);

  static void deallocate(T* ptr);
  static void deallocate(T* ptr, size_type n);

  static void construct(T* ptr);
  static void construct(T* ptr, const T& value);
  static void construct(T* ptr, T&& value);

  template <class... Args>
  static void construct(T* ptr, Args&& ...args);

  static void destroy(T* ptr);
  static void destroy(T* first, T* last);
};

template <class T>
T* allocator<T>::allocate()
{//申请一个T对象大小的空间
  //对于operator new来说，分为全局重载和类重载，全局重载是void* ::operator new(size_t size)，
  //在类中重载形式 void* A::operator new(size_t size)。
  //事实上系统默认的全局::operator new(size_t size)也只是调用malloc分配内存，并且返回一个void* 指针。
  //而构造函数的调用(如果需要)是在new运算符中完成的；
  return static_cast<T*>(::operator new(sizeof(T)));//分配对象T大小的空间，返回空指针并强制转换成T类型指针
}

template <class T>
T* allocator<T>::allocate(size_type n)
{//申请n个T对象大小的空间
  if (n == 0)
    return nullptr;
  return static_cast<T*>(::operator new(n * sizeof(T)));
}

template <class T>
void allocator<T>::deallocate(T* ptr)
{//释放ptr所指向的内存
  if (ptr == nullptr)
    return;
  ::operator delete(ptr);
}

template <class T>
void allocator<T>::deallocate(T* ptr, size_type /*size*/)
{
    //malloc / free的实现在分配时记住了每一个内存块的大小，因此，在释放时不需提醒释放的内存块的大小。 
    //比如你要malloc分配200个字节大小的内存块，malloc实际从操作系统分配了204个字节的内存块，把内存块大小存储在前4个字节，
    //并返回偏移量 + 4的指针。在free时，从指针的 - 4偏移量读4个字节作为内存块的大小，然后释放。
  if (ptr == nullptr)
    return;
  ::operator delete(ptr);
}

template <class T>
void allocator<T>::construct(T* ptr)
{
  mystl::construct(ptr);
}

template <class T>
void allocator<T>::construct(T* ptr, const T& value)
{
  mystl::construct(ptr, value);
}

template <class T>
 void allocator<T>::construct(T* ptr, T&& value)
{
  mystl::construct(ptr, mystl::move(value));
}

template <class T>
template <class ...Args>
 void allocator<T>::construct(T* ptr, Args&& ...args)
{
  mystl::construct(ptr, mystl::forward<Args>(args)...);//这里的...要放在外面，因为forward只能转发一个参数
}

template <class T>
void allocator<T>::destroy(T* ptr)
{
  mystl::destroy(ptr);
}

template <class T>
void allocator<T>::destroy(T* first, T* last)
{
  mystl::destroy(first, last);
}

} // namespace mystl
#endif // !MYTINYSTL_ALLOCATOR_H_

