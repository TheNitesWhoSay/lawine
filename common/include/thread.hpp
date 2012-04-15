/************************************************************************/
/* File Name   : thread.hpp                                             */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 15th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DThread class declaration                              */
/************************************************************************/

#ifndef __SD_COMMON_THREAD_HPP__
#define __SD_COMMON_THREAD_HPP__

/************************************************************************/

#include <common.h>

#ifndef _WIN32
#include <pthread.h>
#endif

/************************************************************************/

class DThread {

public:

	enum RETURN_VALUE {
		RV_SUCCESS	= 0,
		RV_ERROR	= 1,
		RV_PARAM	= 2,
		RV_UNKNOWN	= 3,
	};

public:

	DThread();

	INT GetPriority(VOID) CONST;
	VOID SetPriority(INT priority);

	BOOL Run(VPTR param);
	BOOL Terminate(VOID);
	VOID Wait(VOID);
	INT GetReturn(VOID);

	virtual BOOL Process(VPTR param) = 0;

protected:

#ifdef _WIN32
	HANDLE		m_Thread;
#else
	pthread_t	m_Thread;
#endif

};

/************************************************************************/

#endif	/* __SD_COMMON_THREAD_HPP__ */
