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
//���ڵ���׷�ݵ�trace log
inline static void __trace_debug(const char* function,
	const char* filename,int line,char* format, ...)
{
#ifdef __DEBUG__
	//������ú�������Ϣ
	fprintf(stdout,"��%s:%d��%s",GetFileName(filename).c_str(),line,function);
	//����û����trace��Ϣ
	va_list args;
	va_start(args,format);
	vfprintf(stdout,format,args);
	va_end(args);
#endif
}
#define __TRACE_DEBUG(...) \
__trace_debug(__FUNCTION__,__FILE__,__LINE__,__VA_ARGS__);


typedef void (*__FUNC)();

//һ���ռ�������
template <int inst>
class __MallocAllocTemplate 
{
public:
	static void * allocate(size_t n)
	{
		//����ռ�
		__TRACE_DEBUG("һ���ռ�����������ռ� %d\n",n);
		void * result  =malloc(n);
		if(result == 0)//���벻�ɹ�
		{
			result =  oom_malloc(n);//���õ���   oom_malloc����  
		}
		return   result;
	}

	static void deallocate(void *p, size_t /* n */)
	{
		__TRACE_DEBUG("һ���ռ��������ͷ�\n");
		free(p);
	}
private:
	//����������� �ڴ治������
	static void *  oom_malloc(size_t  n)
	{
		__TRACE_DEBUG("һ���ռ��������ռ���䲻�㡢�ռ��ͷ�\n");
		__FUNC  my_malloc_handler;//��ʾ�����ڴ��ͷŵĺ���ָ��
		
		void * result;
		while(1)
		{
			my_malloc_handler= __malloc_alloc_oom_handler;//��ʵ�ֵ��ڴ��ͷź������˺���ָ��
			if(my_malloc_handler == 0)//Ҫ��   ����ָ��Ϊ�յĻ� �����������쳣
			{
				throw bad_cast();
			}
			my_malloc_handler();//�����������  
			result  = malloc(n);//��������ռ� 
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


//�����ռ�������
template <bool threads, int inst>
class __DefaultAllocTemplate 
{

	enum{__ALIGN = 8};//С��������ϵ��ı߽�
	enum{__MAX_BYTES = 128};//��ʾ����С�����������
	enum {__NFREELISTS = __MAX_BYTES/__ALIGN};//��ʾ����������ĳ��� 
public:
	static void * allocate(size_t n)
	{
		__TRACE_DEBUG("�����ռ�������������ռ�%d\n",n);
		//Ҫ�ǿ��ٵĿռ����128 �ֽڵĻ���������ô��Ҫ���õ��� һ��������
		if(n > (size_t) __MAX_BYTES)
		{
			return __MallocAllocTemplate<0>::allocate(n);
		}
		//����������������
		obj *  result;
		size_t  idex = FREELIST_INDEX(n);//�õ����ڴ���� ���������е�λ�� 
		result  = free_list[idex];//�õ���������ı�����ڴ� ���ַ
		if(result == 0)//��ʾ���������� û��   �ڴ��
		{
			
			return refill(ROUND_UP(n));//���ڴ�����з�  �ڴ�� 
		}
		else
		{
			__TRACE_DEBUG("��������ȡ�ڴ�:_freeList[%d]\n",idex);
			free_list[idex] = result->free_list_link;
			return  result;
		}
	}
	static void * refill(size_t  n)
	{
		__TRACE_DEBUG("����������û��Ϊ%d���ڴ�飬��Ҫ��������ڴ�����зִ�СΪ%d�ڴ�� \n",n,n);
		size_t  nobjs =20;//��ʾҪ���ڴ���� �зֵ� ����
		char*  chunk  =  chunk_alloc(n,nobjs);
		if(nobjs == 1)//��ʾ����ֻ�з��� һ�飻
		{
			return (void *) chunk;
		}
		int idex = FREELIST_INDEX(n);//�õ�����������±�
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
	//���ڴ���� �з�     size ��ʾҪ  �зֵ��ڴ��Ĵ�С   ����nobjs��ʾ���� �зֵĿ��� 
	static char * chunk_alloc(size_t size ,size_t  &  nobjs)
	{
		char * ret;
		size_t total_bytes = size * nobjs;//��ʾ�ܹ�Ҫ�зֵĴ�С
		size_t bytes_left = end_free - start_free;//��ʾ���� ��ǰ�ڴ�صĴ�С 
		if(bytes_left >= total_bytes)//�����ǰ���ڴ�ؿ����зֵ���ô����ڴ�
		{
			__TRACE_DEBUG("�ڴ�����ڴ��㹻����%d������\n",nobjs);
			ret =  start_free;
			start_free +=  total_bytes;//�зֺ���ڴ�صĿ�ʼ��ַ
			return ret;
		}
		else if(bytes_left >= size)//��ʾ�����з�20��
		{

			ret = start_free;
			__TRACE_DEBUG("�ڴ�����ڴ治������%d������ֻ�ܷ���%d������\n",nobjs,bytes_left/size);
			nobjs = bytes_left/size;//��ʾ�зֵĿ��� 
			start_free += (size*nobjs);
			return ret;
		}
		else//��ʾ���� ��ǰ�ڴ��һ���������з� 
		{
			if(bytes_left != 0)//��ʾ���� ��ǰ���ڴ�صĴ�С��Ϊ 0��
			{
				//Ҫ�뽫��ǰ���ڴ�ص��ϵ��ڴ��   ���ʵ��зֵ�   ���������У�
				int idex =  FREELIST_INDEX(bytes_left);
				((obj*)start_free)->free_list_link = free_list[idex];
				free_list[idex] = (obj*)start_free;
				__TRACE_DEBUG("���ڴ����ʣ��Ŀռ䣬�����freeList[%d]\n",idex);
			}
			size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
			start_free = (char *)malloc(bytes_to_get);
			__TRACE_DEBUG("�ڴ�ؿռ䲻�㣬ϵͳ�ѷ���%u bytes�ڴ�\n",bytes_to_get);
			if(start_free ==0)//Ҫ������ʧ�ܵĻ�
			{
				__TRACE_DEBUG("ϵͳ�������㹻������֮�£����ܵ����������п���\n");
				for(size_t i = FREELIST_INDEX(size);i < __NFREELISTS;++i)
				{
					if(free_list[i] != 0)
					{
						start_free = (char  * )free_list[i];
						end_free = start_free + (i +1)*__ALIGN; 
						return chunk_alloc(size,nobjs);
					}
					end_free = 0;
					__TRACE_DEBUG("ϵͳ�Ѻ��������������ڴ棬һ�������������һ������\n");
					start_free = (char*)__MallocAllocTemplate<0>::allocate(bytes_to_get);
				}
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
			__MallocAllocTemplate<0>::deallocate(p);
			return  ;
		}
		int  idex    = FREELIST_INDEX(n);
		obj* q =  (obj*)p;
		q->free_list_link =  free_list[idex];
		free_list[idex] = q;
		

	}

private:
	//��bytes�ϵ���8�ı���
	static size_t ROUND_UP(size_t bytes) 
	{

		return (((bytes) + __ALIGN-1) & ~(__ALIGN - 1));
	}
	//�õ���������������±�
	static  size_t FREELIST_INDEX(size_t  n)
	{
		return   ((n + __ALIGN-1)/__ALIGN -1);
	}
private:
	union obj {
		union obj * free_list_link;//��������
		char client_data[1];    /* The client sees this.        */
	}; 
	static obj *  free_list[__NFREELISTS]; //��������
	static char *start_free;//��ʾ���� �ڴ�ص� ��ʼ
	static char *end_free;//�ڴ�ص�    ����  
	static size_t heap_size;//���ٵ��ڴ�Ĵ�С 
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