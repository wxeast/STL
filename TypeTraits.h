#pragma once

//���͵���ȡ
//�������ֵ��� �������� �� �Զ�������� 


//��������������ʾ    �������� ��  
struct  __FalseType{};//�Զ�������
struct	__TrueType{};//��������

//ʹ�õ���ģ������ʾ
template<class T>
class TypeTraits
{
	typedef __FalseType  IsPODType;
};


//ʹ�õ���ģ����ػ�����ʾ��������
template<>
class TypeTraits<int>
{
	typedef __TrueType   IsPODType;
};

template<>
class TypeTraits<char>
{
	typedef __TrueType   IsPODType;
};
template<>
class TypeTraits<bool>
{
	typedef __TrueType   IsPODType;
};
template<>
class TypeTraits<double>
{
	typedef __TrueType   IsPODType;
};
template<>
class TypeTraits<long>
{
	typedef __TrueType   IsPODType;
};
