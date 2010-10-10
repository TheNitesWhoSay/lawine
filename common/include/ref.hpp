/************************************************************************/
/* File Name   : ref.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 9th, 2009                                          */
/* Module      : Common library                                         */
/* Descript    : DRef class declaration                                 */
/************************************************************************/

#ifndef __SD_COMMON_REF_HPP__
#define __SD_COMMON_REF_HPP__

/************************************************************************/

#include <common.h>

/************************************************************************/

class DRef {

public:

	DRef() : m_RefCounter(0) {}

	INT GetRef(VOID) CONST { return m_RefCounter; }
	INT IncRef(VOID) { return ++m_RefCounter; }
	INT DecRef(VOID) { return --m_RefCounter; }
	VOID ResetRef(VOID) { m_RefCounter = 0; }

private:

	INT		m_RefCounter;

};

/************************************************************************/

#endif	/* __SD_COMMON_REF_HPP__ */
