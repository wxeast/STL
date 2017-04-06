#pragma  once

//迭代器萃取
//用来区分迭代器的类型 

//主要使用的是 五种型别来去区分

//定义五种 迭代器
struct InputIteratorTag {};//只读迭代器
struct OutputIteratorTag {};//只写迭代器 
struct ForwardIteratorTag : public InputIteratorTag {};//单向迭代器
struct BidirectionalIteratorTag : public ForwardIteratorTag {};//双向迭代器
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};//随机迭代器 


//////////////////////////////////////////////////////////////////////////////////////////
//定义一个Iterator 来得到每个迭代器的特性
template <class Iterator>
struct IteratorTraits
{
	//五种特性
	typedef typename InputIteratorTag			IteratorCategory;//迭代器的分类  在这里使用的是一个切片来
																//既可以用来表示单向迭代器，也可以表示的是双向迭代器
	typedef typename Iterator::ValueType		ValueType;//迭代器的内嵌类型
	typedef typename Iterator::DifferenceType	DifferenceType;//迭代器的距离的最大容量
	typedef typename Iterator::Pointer			Pointer;//迭代器的内嵌指针(迭代器所指指的对象)
	typedef typename Iterator::Reference		Reference;//迭代器的内嵌的内容
};

//对于原生指针的一个偏特化
template<class T>
struct IteratorTraits<T*>
{
	typedef typename RandomAccessIteratorTag	 IteratorCategory;//迭代器的分类
	typedef typename T							 ValueType;//迭代器的内嵌类型
	typedef typename size_t						 DifferenceType;//迭代器的距离的最大容量
	typedef typename T*							 Pointer;//迭代器的内嵌指针(迭代器所指指的对象)
	typedef typename T&							 Reference;//迭代器的内嵌的内容

};
template<class T>
struct IteratorTraits<const T*>
{
	typedef typename RandomAccessIteratorTag	 IteratorCategory;//迭代器的分类
	typedef typename T							 ValueType;//迭代器的内嵌类型
	typedef typename size_t						 DifferenceType;//迭代器的距离的最大容量
	typedef typename const T*							 Pointer;//迭代器的内嵌指针(迭代器所指指的对象)
	typedef typename const T&							 Reference;//迭代器的内嵌的内容
};


//////////////////////////////////////////////////////////////////////////////////////////
//定义一个模板函数来得根据 模板类型的参数的不同		来得到两个迭代器之间的对象的个数


//表示的是普通迭代器    求两个迭代器之间的对象的个数
//时间复杂度为 O(n)
template <class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType
	__Distance(InputIterator first, InputIterator last, InputIteratorTag)
{
	IteratorTraits<InputIterator>::DifferenceType count = 0;
	while (first != last) 
	{
		++first; 
		++count;
	}
	return count;
}
//表示的原生指针来求两个迭代器之间的对象个数
//时间复杂度为O(1)
template <class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType
	__Distance(InputIterator first, InputIterator last, RandomAccessIteratorTag)
{
	return last-first;
}
template <class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType
			Distance(InputIterator first, InputIterator last)
{
	return __Distance(first, last, IteratorTraits<InputIterator>::IteratorCategory());
}


//////////////////////////////////////////////////////////////////////////////////////////

template <class InputIterator, class Distance>
inline void __advance(InputIterator& i, Distance n, 
	RandomAccessIteratorTag) {
		i += n;
}

template <class InputIterator, class Distance>
inline void __advance(InputIterator& i, Distance n, 
	InputIteratorTag) {
		while (n--) ++i;
}
template <class InputIterator, class Distance>
inline void advance(InputIterator& i, Distance n) {
	__advance(i, n, IteratorTraits<InputIterator>::IteratorCategory());
}

