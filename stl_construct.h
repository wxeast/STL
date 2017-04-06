#pragma  once
//��������+ �Զ������� 
#include"TypeTraits.h"//����������ȡ  https://github.com/wxeast/STL/blob/master/TypeTraits.h
//��ͨ������+ԭ��������
#include"stl_iterator.h" //����������ȡ https://github.com/wxeast/STL/blob/master/stl_iterator.h

//����������Ļ�������
//��ʹ�ÿռ����������ɵĿռ�������   ��ʼ��   ��   �ռ�����  

///////////////////////////////////////////////////////////////
//�����캯��
//T1��ʾ����   ���ٵĿռ������   T2��ʾ���ǿ��ٵĿռ����������
template<class T1,class T2>
inline void  Construct(T1*  p,const T2& value)
{
	new(p)T1(value);//new�Ķ�λ���ʽ
};



/////////////////////////////////////////////////
//���������ʱ��ֱ�ӵ���������
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

//��������ʱ��   ʹ�õ���������ȡ�����������������Զ�������
template<class ForWardIterator>
inline void  Destroy(ForWardIterator first, ForWardIterator  last)
{
	//�жϸõ���������Ƕ������     �������ͻ����Զ�������
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









