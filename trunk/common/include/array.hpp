/************************************************************************/
/* File Name   : array.hpp                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : April 21st, 2011                                       */
/* Module      : Common library                                         */
/* Descript    : DArray class declaration                               */
/************************************************************************/

#ifndef __SD_COMMON_ARRAY_HPP__
#define __SD_COMMON_ARRAY_HPP__

/************************************************************************/

#include <common.h>

/************************************************************************/

template<typename T>
class DArray {

public:

	typedef	T		*TPTR;
	typedef CONST T	*TCPTR;

	DArray();
	explicit DArray(UINT count);
	DArray(TCPTR buf, UINT count);
	~DArray();

	BOOL IsNull(VOID) CONST;
	UINT GetCount(VOID) CONST;
	TCPTR GetBuffer(VOID) CONST;
	TPTR GetBuffer(VOID);

	TPTR Alloc(UINT count);
	VOID Free(VOID);

	operator TPTR ();
	operator TCPTR () CONST;

protected:

	UINT		m_Count;
	TPTR		m_Buffer;

private:

	explicit DArray(CONST DArray &buf);
	DArray &operator = (CONST DArray &buf);

};

/************************************************************************/

#include "array.inl"

/************************************************************************/

#endif	/* __SD_COMMON_ARRAY_HPP__ */
