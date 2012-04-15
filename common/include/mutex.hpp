/************************************************************************/
/* File Name   : mutex.hpp                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 18th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DMutex class declaration                               */
/************************************************************************/

#ifndef __SD_COMMON_MUTEX_HPP__
#define __SD_COMMON_MUTEX_HPP__

/************************************************************************/

#include <common.h>

#ifndef _WIN32
#include <pthread.h>
#endif

/************************************************************************/

class DMutex {

public:

	DMutex();
	~DMutex();

	BOOL Lock(BOOL wait = TRUE, DWORD timeout = 0UL);
	BOOL Unlock(VOID);

protected:

#ifdef _WIN32
	CRITICAL_SECTION	m_Lock;
#else
	pthread_mutex_t		m_Lock;
#endif

};

/************************************************************************/

class DAutoLock {

public:

	explicit DAutoLock(DMutex &lock, DWORD timeout = 0UL);
	~DAutoLock();

protected:

	DMutex	&m_Lock;

};

/************************************************************************/

#endif	/* __SD_COMMON_MUTEX_HPP__ */
