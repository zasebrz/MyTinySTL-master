#ifndef MYTINYSTL_CONSTRUCT_H_
#define MYTINYSTL_CONSTRUCT_H_

// 这个头文件包含两个函数 construct，destroy
// construct : 负责对象的构造，这是在申请内存之后的操作
// destroy   : 负责对象的析构，先析构再释放内存

#include <new>

#include "type_traits.h"
#include "iterator.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)  // unused parameter
#endif // _MSC_VER

namespace mystl
{

// construct 构造对象

template <class Ty>
void construct(Ty* ptr)
{
  ::new ((void*)ptr) Ty();//ptr是个对象指针，先转成无类型指针，
                          //::new表示placement new意即“放置”，这种new允许在一块已经分配成功的内存上重新构造对象或对象数组。
                          //placement new，不用担心内存分配失败，因为它根本不分配内存，它做的唯一一件事情就是调用对象的构造函数。
                          //调用方法是 ::new (ptr) class();一般来说ptr可以是任意指针，他只用来表示地址，后面的class()其实表示值初始化
                          //只不过没有传入值，所以调用默认构造函数（合成的或者自定义的都行）
}

template <class Ty1, class Ty2>
void construct(Ty1* ptr, const Ty2& value)//构造对象并初始化对象，直接调用对应的构造函数，这里只传入了一个形参
{
  ::new ((void*)ptr) Ty1(value);
}

template <class Ty, class... Args>
void construct(Ty* ptr, Args&&... args)//构造对象并初始化对象，直接调用对应的构造函数，这里传入了多个形参
{
  ::new ((void*)ptr) Ty(mystl::forward<Args>(args)...);
}

// destroy 将对象析构

template <class Ty>
void destroy_one(Ty*, std::true_type) {}//啥也不做？

template <class Ty>
void destroy_one(Ty* pointer, std::false_type)
{
  if (pointer != nullptr)
  {//非空显示调用析构函数
    pointer->~Ty();
  }
}

template <class ForwardIter>
void destroy_cat(ForwardIter , ForwardIter , std::true_type) {}

template <class ForwardIter>
void destroy_cat(ForwardIter first, ForwardIter last, std::false_type)
{
  for (; first != last; ++first)
    destroy(&*first);//first是迭代器，所以先要取出元素，然后再取地址（不能直接取得迭代器的底层指针？）
}

template <class Ty>
void destroy(Ty* pointer)
{
  destroy_one(pointer, std::is_trivially_destructible<Ty>{});//对于那些自定义析构函数（非虚）的类，才能去主动调用析构函数析构，
  //否则的话就不能主动析构
}

template <class ForwardIter>
void destroy(ForwardIter first, ForwardIter last)//将迭代器指向的对象先判断一下是不是能主动析构，
{
  destroy_cat(first, last, std::is_trivially_destructible<
              typename iterator_traits<ForwardIter>::value_type>{});
}

} // namespace mystl

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // !MYTINYSTL_CONSTRUCT_H_

