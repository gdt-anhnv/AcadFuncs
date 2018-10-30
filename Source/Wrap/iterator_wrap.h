#ifndef _ITERATOR_WRAP_H_
#define _ITERATOR_WRAP_H_

#include "../acad_header.h"

template <typename T>
struct IteratorWrap
{
	T* pointer;

	IteratorWrap(T*);
	~IteratorWrap();

	IteratorWrap(const IteratorWrap&);
	IteratorWrap();

private:
	const IteratorWrap& operator=(const IteratorWrap&);
};


template<typename T>
IteratorWrap<T>::IteratorWrap(T * t) :
	pointer(t)
{
}

template<typename T>
IteratorWrap<T>::~IteratorWrap()
{
	if (NULL != pointer)
		delete pointer;
}

template<typename T>
IteratorWrap<T>::IteratorWrap(const IteratorWrap & iter_wrap) :
	pointer(iter_wrap.pointer)
{
}

template<typename T>
IteratorWrap<T>::IteratorWrap() :
	pointer(NULL)
{
}

#endif 