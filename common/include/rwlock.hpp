/************************************************************************/
/* File Name   : rwlock.hpp                                             */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 18th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DRwLock class declaration                              */
/************************************************************************/

#ifndef __SD_COMMON_RWLOCK_HPP__
#define __SD_COMMON_RWLOCK_HPP__

/************************************************************************/

#include <common.h>

#ifndef _WIN32
#include <pthread.h>
#endif

/************************************************************************/

class DRwLock {

public:

	DRwLock();
	~DRwLock();

	BOOL Read(BOOL wait = TRUE, UINT timeout = 0U) CONST;
	BOOL Write(BOOL wait = TRUE, UINT timeout = 0U);
	BOOL Unlock(VOID) CONST;

protected:

#ifdef _WIN32
	mutable BOOL				m_RW;
	mutable CRITICAL_SECTION	m_RdLock;
	mutable CRITICAL_SECTION	m_WrLock;
#else
	mutable pthread_rwlock_t	m_Lock;
#endif
};

/************************************************************************/

#endif	/* __SD_COMMON_RWLOCK_HPP__ */
