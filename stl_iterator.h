#pragma  once

//��������ȡ
//�������ֵ����������� 

//��Ҫʹ�õ��� �����ͱ���ȥ����

//�������� ������
struct InputIteratorTag {};//ֻ��������
struct OutputIteratorTag {};//ֻд������ 
struct ForwardIteratorTag : public InputIteratorTag {};//���������
struct BidirectionalIteratorTag : public ForwardIteratorTag {};//˫�������
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};//��������� 


//////////////////////////////////////////////////////////////////////////////////////////
//����һ��Iterator ���õ�ÿ��������������
template <class Iterator>
struct IteratorTraits
{
	//��������
	typedef typename InputIteratorTag			IteratorCategory;//�������ķ���  ������ʹ�õ���һ����Ƭ��
																//�ȿ���������ʾ�����������Ҳ���Ա�ʾ����˫�������
	typedef typename Iterator::ValueType		ValueType;//����������Ƕ����
	typedef typename Iterator::DifferenceType	DifferenceType;//�������ľ�����������
	typedef typename Iterator::Pointer			Pointer;//����������Ƕָ��(��������ָָ�Ķ���)
	typedef typename Iterator::Reference		Reference;//����������Ƕ������
};

//����ԭ��ָ���һ��ƫ�ػ�
template<class T>
struct IteratorTraits<T*>
{
	typedef typename RandomAccessIteratorTag	 IteratorCategory;//�������ķ���
	typedef typename T							 ValueType;//����������Ƕ����
	typedef typename size_t						 DifferenceType;//�������ľ�����������
	typedef typename T*							 Pointer;//����������Ƕָ��(��������ָָ�Ķ���)
	typedef typename T&							 Reference;//����������Ƕ������

};
template<class T>
struct IteratorTraits<const T*>
{
	typedef typename RandomAccessIteratorTag	 IteratorCategory;//�������ķ���
	typedef typename T							 ValueType;//����������Ƕ����
	typedef typename size_t						 DifferenceType;//�������ľ�����������
	typedef typename const T*							 Pointer;//����������Ƕָ��(��������ָָ�Ķ���)
	typedef typename const T&							 Reference;//����������Ƕ������
};


//////////////////////////////////////////////////////////////////////////////////////////
//����һ��ģ�庯�����ø��� ģ�����͵Ĳ����Ĳ�ͬ		���õ�����������֮��Ķ���ĸ���


//��ʾ������ͨ������    ������������֮��Ķ���ĸ���
//ʱ�临�Ӷ�Ϊ O(n)
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
//��ʾ��ԭ��ָ����������������֮��Ķ������
//ʱ�临�Ӷ�ΪO(1)
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

