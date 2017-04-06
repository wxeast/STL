#pragma  once
//内置类型+ 自定义类型 
#include"TypeTraits.h"//类型类型萃取  https://github.com/wxeast/STL/blob/master/TypeTraits.h
//普通迭代器+原生迭代器
#include"stl_iterator.h" //迭代器的萃取 https://github.com/wxeast/STL/blob/master/stl_iterator.h

//构造和析构的基本工具
//对使用空间配置器生成的空间来进行   初始化   与   空间析构  

///////////////////////////////////////////////////////////////
//调构造函数
//T1表示的是   开辟的空间的类型   T2表示的是开辟的空间的内置类型
template<class T1,class T2>
inline void  Construct(T1*  p,const T2& value)
{
	new(p)T1(value);//new的定位表达式
};



/////////////////////////////////////////////////
//单个对象的时候直接调析构函数
template<class T>
inline void Destroy(T*  p)
{
	p->~T();
}

template<class ForWardIterator>
inline void  _Destroy(ForWardIterator first, ForWardIterator  last,__FalseType)
{
	while(first!= last)
	{
		Destroy(&(*first));
		++first;
	}
}
template<class ForWardIterator>
inline void  _Destroy(ForWardIterator first, ForWardIterator  last,__TrueType)
{}

//多个对象的时候   使用的是类型萃取来区分内置类型与自定义类型
template<class ForWardIterator>
inline void  Destroy(ForWardIterator first, ForWardIterator  last)
{
	//判断该迭代器中内嵌类型是     内置类型还是自定义类型
	typedef IteratorTraits<ForWardIterator>::ValueType  T;
	_Destroy(first,last,TypeTraits<T>::IsPODType);
}


inline void Destroy(char*, char*) {}
inline void Destroy(wchar_t*, wchar_t*) {}
inline void Destroy(int*, int*) {}
template<>
inline void Destroy<string*>(string* first, string* last) 
{
	while(first!= last)
	{
		Destroy(&(*first));
		++first;
	}
}









