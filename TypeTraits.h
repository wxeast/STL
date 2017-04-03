#pragma once

//类型的萃取
//用来区分的是 内置类型 与 自定义的类型 


//定义两个类来表示    两种类型 ：  
struct  __FalseType{};//自定义类型
struct	__TrueType{};//内置类型

//使用的是模板来表示
template<class T>
class TypeTraits
{
	typedef __FalseType  IsPODType;
};


//使用的是模板的特化来表示内置类型
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
