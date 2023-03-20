#ifndef MYTINYSTL_ALLOC_H_
#define MYTINYSTL_ALLOC_H_

// 这个头文件包含一个类 alloc，用于分配和回收内存，以内存池的方式实现
//
// 从 v2.0.0 版本开始，将不再使用内存池，这个文件将被弃用，但暂时保留
//
// 注意！！！
// 我知道这个文件里很多实现是错的，这是很久很久前写的了，后面已经不用这个东西了，
// 所以我也没再维护，有诸多问题，已经有人在issue中都提了，free_list的修改，
// 指针作为参数时没实际修改到原指针，等等。相信会看这么仔细的，大部分都是
// 初学C++的朋友，大佬都不会看这些玩具了，所以其中的错误，就留给对内存池实现
// 感兴趣的朋友去修改啦！

#include <new>

#include <cstddef>
#include <cstdio>

namespace mystl
{

// 共用体: FreeList
// 采用链表的方式管理内存碎片，分配与回收小内存（<=4K）区块
union FreeList
{
  union FreeList* next;  // 指向下一个区块1
  char data[1];          // 储存本块内存的首地址

};

// 不同内存范围的上调大小
//通过EFreeListsNumber个链表来管理申请的内存。而且分配的倍数是8(_ALIGN)的倍数，就算你申请1byte它也会给你分配8byte。
//56 * 8 = 128(_MAX_BYTES)。所以可以看到这16个链表节点分别是大小为8，16，24，... ... ，128bytes.
enum
{
  EAlign128 = 8, 
  EAlign256 = 16, 
  EAlign512 = 32,
  EAlign1024 = 64, 
  EAlign2048 = 128,
  EAlign4096 = 256
};

// 小对象的内存大小 4KB
enum { ESmallObjectBytes = 4096 };

// free lists 个数
enum { EFreeListsNumber = 56 };

// 空间配置类 alloc
// 如果内存较大，超过 4096 bytes，直接调用 std::malloc, std::free
// 当内存较小时，以内存池管理，每次配置一大块内存，并维护对应的自由链表
class alloc
{
private:
  static char*  start_free;                      // 内存池起始位置
  static char*  end_free;                        // 内存池结束位置
  static size_t heap_size;                       // 申请 heap 空间附加值大小
  
  static FreeList* free_list[EFreeListsNumber];  // 自由链表

public:
  static void* allocate(size_t n);
  static void  deallocate(void* p, size_t n);
  static void* reallocate(void* p, size_t old_size, size_t new_size);

private:
  static size_t M_align(size_t bytes);
  static size_t M_round_up(size_t bytes);
  static size_t M_freelist_index(size_t bytes);
  static void*  M_refill(size_t n);
  static char*  M_chunk_alloc(size_t size, size_t &nobj);
};

// 静态成员变量初始化

char*  alloc::start_free = nullptr;
char*  alloc::end_free = nullptr;
size_t alloc::heap_size = 0;

FreeList* alloc::free_list[EFreeListsNumber] = {
  nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
  nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
  nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
  nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
  nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
  nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
  nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
  };//56个小内存的首地址？

// 分配大小为 n 字节的空间， n > 0
inline void* alloc::allocate(size_t n)
{
  FreeList* my_free_list;
  FreeList* result;
  if (n > static_cast<size_t>(ESmallObjectBytes))
    return std::malloc(n);//如果申请的内存过大，利用malloc分配
  my_free_list = free_list[M_freelist_index(n)];// 根据区块大小，选择第 n 个 free lists
  //my_free_list就是这个内存链表的头结点
  result = my_free_list;//先记录头结点，再顺着链表往后找
  if (result == nullptr)
  {//头结点为空，说明之前还没分配过这个多大小的内存
    void* r = M_refill(M_round_up(n));
    return r;
  }
  my_free_list = result->next;//似乎没什么用
  return result;
}

// 释放 p 指向的大小为 n 的空间, p 不能为 0
inline void alloc::deallocate(void* p, size_t n)
{
  if (n > static_cast<size_t>(ESmallObjectBytes))
  {
    std::free(p);
    return;
  }
  FreeList* q = reinterpret_cast<FreeList*>(p);
  FreeList* my_free_list;
  my_free_list = free_list[M_freelist_index(n)];
  q->next = my_free_list;
  my_free_list = q;
}

// 重新分配空间，接受三个参数，参数一为指向新空间的指针，参数二为原来空间的大小，参数三为申请空间的大小
inline void* alloc::reallocate(void* p, size_t old_size, size_t new_size)
{
  deallocate(p, old_size);
  p = allocate(new_size);
  return p;
}

// bytes 对应上调大小
inline size_t alloc::M_align(size_t bytes)
{
  if (bytes <= 512)
  {
    return bytes <= 256
      ? bytes <= 128 ? EAlign128 : EAlign256
      : EAlign512;
  }
  return bytes <= 2048
    ? bytes <= 1024 ? EAlign1024 : EAlign2048
    : EAlign4096;
}

// 将 bytes 上调至对应区间大小
inline size_t alloc::M_round_up(size_t bytes)
{//比如申请1，就分配8，申请12，就分配16，申请157，就分配160...
    //https://blog.csdn.net/weixin_45768137/article/details/127266001
  return ((bytes + M_align(bytes) - 1) & ~(M_align(bytes) - 1));
}

// 根据区块大小，选择第 n 个 free lists
inline size_t alloc::M_freelist_index(size_t bytes)
{
  if (bytes <= 512)
  {
    return bytes <= 256
      ? bytes <= 128 
        ? ((bytes + EAlign128 - 1) / EAlign128 - 1) 
        //这里返回的是链表头在数组free_list中的下标，[0:128]的时候会用 8 的倍数去分配，
        //因为下标是0开始的，所以最后还要-1。如要分配是22byte，因为实际应该分配24（3*8）byte，
        //它的链表下标是2（下标0对应8byte，下标1对应16byte，2-24，最大下标是15），所以是（22+8-1）/ 8 -1=2
        : (15 + (bytes + EAlign256 - 129) / EAlign256)
        //[129:256]的时候会用 16 的倍数去分配，如要分配是156byte，因为实际应该分配 160（128+16*2）byte，
        //它的链表下标是17（下标0-15是8倍内存，16-23是16倍内存），所以前面要加15，减去的起始值是129，注意这里不用-1
        //因为15里面已经减去了，所以是15+（156+16-129）/ 16=17
      : (23 + (bytes + EAlign512 - 257) / EAlign512);
      //[257:512]的时候会用 32 的倍数去分配，如要分配是 347 byte，因为实际应该分配 352（256+32*3）byte，
      //它的链表下标是26（下标0-15是8倍内存，16-23是16倍内存,24-31是32倍内存），所以前面要加23，减去的起始值是257
      //所以是23+（347+32-257）/ 32=26
  }
  return bytes <= 2048
    ? bytes <= 1024 //下标32-39是64倍内存，40-47是128倍内存,48-55是512倍内存
      ? (31 + (bytes + EAlign1024 - 513) / EAlign1024)
      : (39 + (bytes + EAlign2048 - 1025) / EAlign2048)
    : (47 + (bytes + EAlign4096 - 2049) / EAlign4096);
}

// 重新填充 free list
void* alloc::M_refill(size_t n)
{//如果想申请n字节的空间，会直接分配1个或多个n字节大小的区块
  size_t nblock = 10;
  char* c = M_chunk_alloc(n, nblock);
  FreeList* my_free_list;
  FreeList* result, *cur, *next;
  // 如果只有一个区块，就把这个区块返回给调用者，free list 没有增加新节点
  if (nblock == 1)
    return c;
  // 否则把一个区块给调用者，剩下的纳入 free list 作为新节点
  my_free_list = free_list[M_freelist_index(n)];
  result = (FreeList*)c;//仍然是返回第一个区块
  my_free_list = next = (FreeList*)(c + n);//c + n是第一个区块空间的下一个地址，也就是第二个区块的起始地址
  for (size_t i = 1; ; ++i)
  {//逐个放入链表中
    cur = next;
    next = (FreeList*)((char*)next + n);//下一个区块的起始地址
    if (nblock - 1 == i)
    {//cur已经是最后一个区块了
      cur->next = nullptr;
      break;
    }
    else
    {//链接起来
      cur->next = next;
    }
  }
  return result;
}

// 从内存池中取空间给 free list 使用，条件不允许时，会调整 nblock
char* alloc::M_chunk_alloc(size_t size, size_t& nblock)
{//整个内存分配的逻辑是：先向内存池申请内存，不够的话向堆申请，堆不够的话，在空闲链表上找，这个链表是用来存储内存池未用的所有
    //内存块的，有可能之前剩余了一个非常大的内存块，可以满足这一次需求
    //堆够的话分配两倍大小的空间，再分配给用户。内存池够得话，直接分配给用户
  char* result;
  size_t need_bytes = size * nblock;//最好是直接分配10个区块，每一个大小都是n byte
  size_t pool_bytes = end_free - start_free;

  // 如果内存池剩余大小完全满足需求量，返回它
  if (pool_bytes >= need_bytes)
  {
    result = start_free;
    start_free += need_bytes;
    return result;
  }

  // 如果内存池剩余大小不能完全满足需求量，但至少可以分配一个或一个以上的区块，就返回它
  else if (pool_bytes >= size)
  {
    nblock = pool_bytes / size;//能分配几个区块就分配几个
    need_bytes = size * nblock;
    result = start_free;
    start_free += need_bytes;
    return result;
  }

  // 如果内存池剩余大小连一个区块都无法满足
  else
  {
    if (pool_bytes > 0)
    { // 如果内存池还有剩余，把剩余的空间加入到 free list 中
      FreeList* my_free_list = free_list[M_freelist_index(pool_bytes)];//先找到剩余空间能够存放的链表在数组中的下标
      //然后取出头结点my_free_list
      ((FreeList*)start_free)->next = my_free_list;
      //将剩下的空间首地址先强转成一个节点指针，然后插入头部，现在剩余地址成了头结点
      my_free_list = (FreeList*)start_free;
    }
    // 申请 heap 空间
    size_t bytes_to_get = (need_bytes << 1) + M_round_up(heap_size >> 4);
    //M_chunk_alloc的职责就是负责内存池的所有内存的生产，在生产的时候他为了保证下次能有内存用，所以会将空间 * 2去申请
    start_free = (char*)std::malloc(bytes_to_get);
    if (!start_free)
    { // heap 空间也不够
      FreeList* my_free_list, *p;
      // 试着查找有无未用区块，且区块足够大的 free list
      for (size_t i = size; i <= ESmallObjectBytes; i += M_align(i))
      {//size是我们需要的大小，他一定符合标准尺寸，
        my_free_list = free_list[M_freelist_index(i)];//i大小是不小于size的
        p = my_free_list;
        if (p)
        {//有空闲块，那么可以分配给用户
          my_free_list = p->next;//似乎没什么用
          start_free = (char*)p;//把这一块内存重新放入内存池，递归调用本函数去分配
          end_free = start_free + i;
          return M_chunk_alloc(size, nblock);
        }
        //没有空闲块，寻找更大的空间
      }
      //如果前面一直没有返回说明链表里面也没有足够空间
      std::printf("out of memory");
      end_free = nullptr;
      throw std::bad_alloc();//抛出异常
    }
    end_free = start_free + bytes_to_get;//堆空间够得话，直接进入下一层递归分配
    heap_size += bytes_to_get;
    return M_chunk_alloc(size, nblock);
  }
}

} // namespace mystl
#endif // !MYTINYSTL_ALLOC_H_

