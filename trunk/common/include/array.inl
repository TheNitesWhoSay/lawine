/************************************************************************/
/* File Name   : array.cpp                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : April 21st, 201                                        */
/* Module      : Common library                                         */
/* Descript    : DArray class implementation                            */
/************************************************************************/

/************************************************************************/

template<typename T>
DArray<T>::DArray(UINT count) :
	m_Buffer(NULL),
	m_Count(0U)
{
	DAssert(count);

	Alloc(count);
}

template<typename T>
DArray<T>::DArray(TCPTR buf, UINT count) :
	m_Buffer(NULL),
	m_Count(0U)
{
	DAssert(buf && count);

	DVerify(Alloc(count));
	for (UINT i = 0; i < count; i++, buf++)
		m_Buffer[i] = *buf;
}

template<typename T>
DArray<T>::~DArray()
{
	Free();
}

/************************************************************************/

template<typename T>
UINT DArray<T>::GetCount(VOID) CONST
{
	return m_Count;
}

template<typename T>
typename DArray<T>::TCPTR DArray<T>::GetBuffer(VOID) CONST
{
	return m_Buffer;
}

template<typename T>
typename DArray<T>::TPTR DArray<T>::GetBuffer(VOID)
{
	return m_Buffer;
}

template<typename T>
DArray<T>::operator TPTR ()
{
	return m_Buffer;
}

template<typename T>
DArray<T>::operator TCPTR () CONST
{
	return m_Buffer;
}

/************************************************************************/

template<typename T>
typename DArray<T>::TPTR DArray<T>::Alloc(UINT count)
{
	DAssert(!m_Buffer);

	if (!count)
		return NULL;

	m_Buffer = new T[count];
	m_Count = count;
	return m_Buffer;
}

template<typename T>
VOID DArray<T>::Free(VOID)
{
	delete [] m_Buffer;
	m_Buffer = NULL;
	m_Count = 0U;
}

/************************************************************************/

template<typename T>
DArray<T>::DArray(CONST DArray &buf)
{
	DAssert(FALSE);
}

template<typename T>
DArray<T> &DArray<T>::operator = (CONST DArray &buf)
{
	DAssert(FALSE);
	return *this;
}

/************************************************************************/
