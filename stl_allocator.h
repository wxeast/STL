#pragma once
#include<stdarg.h>
#include<string>

//trace
#define  __DEBUG__
static string GetFileName(const string& path)
{
	char  ch= '/';
#ifdef _WIN32
	ch = '\\';
#endif
	size_t pos = path.rfind(ch);
	if(pos==string::npos)
		return path;
	else
		return path.substr(pos+ 1);
}
//用于调试追溯的trace log
inline static void __trace_debug(const char* function,
	const char* filename,int line,char* format, ...)
{
#ifdef __DEBUG__
	//输出调用函数的信息
	fprintf(stdout,"【%s:%d】%s",GetFileName(filename).c_str(),line,function);
	//输出用户打的trace信息
	va_list args;
	va_start(args,format);
	vfprintf(stdout,format,args);
	va_end(args);
#endif
}
#define __TRACE_DEBUG(...) \
__trace_debug(__FUNCTION__,__FILE__,__LINE__,__VA_ARGS__);


typedef void (*__FUNC)();

//一级空间配置器
template <int inst>
class __MallocAllocTemplate 
{
public:
	static void * allocate(size_t n)
	{
		//申请空间
		__TRACE_DEBUG("一级空间配置器申请空间 %d\n",n);
		void * result  =malloc(n);
		if(result == 0)//申请不成功
		{
			result =  oom_malloc(n);//调用的是   oom_malloc函数  
		}
		return   result;
	}

	static void deallocate(void *p, size_t /* n */)
	{
		__TRACE_DEBUG("一级空间配置器释放\n");
		free(p);
	}
private:
	//用来处理的是 内存不足的情况
	static void *  oom_malloc(size_t  n)
	{
		__TRACE_DEBUG("一级空间配置器空间分配不足、空间释放\n");
		__FUNC  my_malloc_handler;//表示的是内存释放的函数指针
		
		void * result;
		while(1)
		{
			my_malloc_handler= __malloc_alloc_oom_handler;//将实现的内存释放函数给此函数指针
			if(my_malloc_handler == 0)//要是   函数指针为空的话 ，，，，抛异常
			{
				throw bad_cast();
			}
			my_malloc_handler();//调用这个函数  
			result  = malloc(n);//重新申请空间 
			if(result)
			{
				return  result;
			}
		}
	}
	static void (*__malloc_alloc_oom_handler)();
};

template<int inst >
void (* __MallocAllocTemplate<inst>::__malloc_alloc_oom_handler)() = 0;


//二级空间配置器
template <bool threads, int inst>
class __DefaultAllocTemplate 
{

	enum{__ALIGN = 8};//小型区域的上调的边界
	enum{__MAX_BYTES = 128};//表示的是小型区域的上限
	enum {__NFREELISTS = __MAX_BYTES/__ALIGN};//表示的自由链表的长度 
public:
	static void * allocate(size_t n)
	{
		__TRACE_DEBUG("二级空间配置器、申请空间%d\n",n);
		//要是开辟的空间大于128 字节的话，，，那么就要调用的是 一级配置器
		if(n > (size_t) __MAX_BYTES)
		{
			return __MallocAllocTemplate<0>::allocate(n);
		}
		//从自由链表中来找
		obj *  result;
		size_t  idex = FREELIST_INDEX(n);//得到该内存块在 自由链表中的位置 
		result  = free_list[idex];//得到自由链表的保存的内存 块地址
		if(result == 0)//表示自由链表中 没有   内存块
		{
			
			return refill(ROUND_UP(n));//从内存池中切分  内存块 
		}
		else
		{
			__TRACE_DEBUG("自由链表取内存:_freeList[%d]\n",idex);
			free_list[idex] = result->free_list_link;
			return  result;
		}
	}
	static void * refill(size_t  n)
	{
		__TRACE_DEBUG("自由链表中没有为%d的内存块，需要从狭义的内存池中切分大小为%d内存块 \n",n,n);
		size_t  nobjs =20;//表示要从内存池中 切分的 块数
		char*  chunk  =  chunk_alloc(n,nobjs);
		if(nobjs == 1)//表示的是只切分了 一块；
		{
			return (void *) chunk;
		}
		int idex = FREELIST_INDEX(n);//得到自由链表的下标
		obj*  cur  = (obj *)(chunk + n);
		free_list[idex] = cur;
		for(size_t   i = 2;i < nobjs;++i )
		{
			obj *  next  = (obj * )(chunk + i* n);
			cur->free_list_link = next;
			cur = next;
		}
		cur->free_list_link = 0;
		return  (void *)chunk;
	}
	//从内存池中 切分     size 表示要  切分的内存块的大小   ，，nobjs表示的是 切分的块数 
	static char * chunk_alloc(size_t size ,size_t  &  nobjs)
	{
		char * ret;
		size_t total_bytes = size * nobjs;//表示总共要切分的大小
		size_t bytes_left = end_free - start_free;//表示的是 当前内存池的大小 
		if(bytes_left >= total_bytes)//如果当前的内存池可以切分到这么大的内存
		{
			__TRACE_DEBUG("内存池中内存足够分配%d个对象\n",nobjs);
			ret =  start_free;
			start_free +=  total_bytes;//切分后的内存池的开始地址
			return ret;
		}
		else if(bytes_left >= size)//表示不够切分20块
		{

			ret = start_free;
			__TRACE_DEBUG("内存池中内存不够分配%d个对象，只能分配%d个对象\n",nobjs,bytes_left/size);
			nobjs = bytes_left/size;//表示切分的块数 
			start_free += (size*nobjs);
			return ret;
		}
		else//表示的是 当前内存池一个都不够切分 
		{
			if(bytes_left != 0)//表示的是 当前的内存池的大小不为 0；
			{
				//要想将当前的内存池的上的内存块   合适的切分到   自由链表中；
				int idex =  FREELIST_INDEX(bytes_left);
				((obj*)start_free)->free_list_link = free_list[idex];
				free_list[idex] = (obj*)start_free;
				__TRACE_DEBUG("将内存池中剩余的空间，分配给freeList[%d]\n",idex);
			}
			size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
			start_free = (char *)malloc(bytes_to_get);
			__TRACE_DEBUG("内存池空间不足，系统堆分配%u bytes内存\n",bytes_to_get);
			if(start_free ==0)//要是申请失败的话
			{
				__TRACE_DEBUG("系统堆已无足够，无奈之下，智能到自由链表中看看\n");
				for(size_t i = FREELIST_INDEX(size);i < __NFREELISTS;++i)
				{
					if(free_list[i] != 0)
					{
						start_free = (char  * )free_list[i];
						end_free = start_free + (i +1)*__ALIGN; 
						return chunk_alloc(size,nobjs);
					}	
				}
				end_free = 0;
				__TRACE_DEBUG("系统堆和自由链表都已无内存，一级配置器做最后一根稻草\n");
				start_free = (char*)__MallocAllocTemplate<0>::allocate(bytes_to_get);
			}
			heap_size += bytes_to_get;
			end_free = start_free + bytes_to_get;
			return(chunk_alloc(size, nobjs));
		}
	}
	static void deallocate(void *p, size_t n)
	{
		if(n > (size_t )__MAX_BYTES)
		{
			__MallocAllocTemplate<0>::deallocate(p,n);
			return  ;
		}
		int  idex    = FREELIST_INDEX(n);
		obj* q =  (obj*)p;
		q->free_list_link =  free_list[idex];
		free_list[idex] = q;
		

	}

private:
	//将bytes上调的8的倍数
	static size_t ROUND_UP(size_t bytes) 
	{

		return (((bytes) + __ALIGN-1) & ~(__ALIGN - 1));
	}
	//得到的是自由链表的下标
	static  size_t FREELIST_INDEX(size_t  n)
	{
		return   ((n + __ALIGN-1)/__ALIGN -1);
	}
private:
	union obj {
		union obj * free_list_link;//自由链表
		char client_data[1];    /* The client sees this.        */
	}; 
	static obj *  free_list[__NFREELISTS]; //自由链表
	static char *start_free;//表示的是 内存池的 开始
	static char *end_free;//内存池的    结束  
	static size_t heap_size;//开辟的内存的大小 
};


template <bool threads, int inst>
char *  __DefaultAllocTemplate<threads,inst>::start_free  = 0;
template <bool threads, int inst>
char *  __DefaultAllocTemplate<threads,inst>::end_free = 0;
template <bool threads, int inst>
size_t  __DefaultAllocTemplate<threads,inst>::heap_size  = 0 ;
template <bool threads, int inst>
typename __DefaultAllocTemplate<threads,inst>::obj* __DefaultAllocTemplate<threads,inst>::free_list[__NFREELISTS] = {0};



#ifdef  __USE_MALLOC
typedef __MallocAllocTemplate<0> alloc;
#else
typedef  __DefaultAllocTemplate<false,0> alloc;
#endif //__USE_MALLOC


template<class T,class  Alloc  = alloc>
class simple_alloc 
{
public :
	static T * allocate(size_t n)
	{
		return 0 ==n ? 0 : (T*)Alloc::allocate(n*sizeof(T));
	}
	static  T* allocate(void)
	{
		return  (T*)Alloc::allocate(sizeof(T));
	}
	static void deallocate(T *p, size_t n)
	{
		if(n!= 0)
			Alloc::deallocate(p,sizeof(T)*n);
	}
	static void deallocate(T *p)
	{
		Alloc::deallocate(p,sizeof(T));
	}
};




#include<vector>
void TestAllocate()
{
	simple_alloc<int ,alloc> sa;
	vector<int*> va;
	for(int i = 0;i <20;i++)
	{
		va.push_back(sa.allocate(2));
		cout<<endl;
	}
	va.push_back(sa.allocate(2));
} 
