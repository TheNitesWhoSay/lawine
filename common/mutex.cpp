/************************************************************************/
/* File Name   : mutex.cpp                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 18th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DMutex class implementation                            */
/************************************************************************/

#include <mutex.hpp>

/************************************************************************/

DMutex::DMutex()
{
#ifdef WIN32
	::InitializeCriticalSection(&m_Lock);
#else
	::pthread_mutex_init(&m_Lock, NULL);
#endif
}

DMutex::~DMutex()
{
#ifdef WIN32
	::DeleteCriticalSection(&m_Lock);
#else
	::pthread_mutex_destroy(&m_Lock);
#endif
}

BOOL DMutex::Lock(BOOL wait /* = TRUE */, DWORD timeout /* = 0UL */)
{
#ifdef WIN32
	::EnterCriticalSection(&m_Lock);
	return TRUE;
#else
	if (!wait)
		return !::pthread_mutex_trylock(&m_Lock);

	if (!timeout)
		return !::pthread_mutex_lock(&m_Lock);

	timespec ts;
	ts.tv_sec = timeout;
	ts.tv_nsec = timeout % 1000 * 1000000;

	return !::pthread_mutex_timedlock(&m_Lock, &ts);
#endif
}

BOOL DMutex::Unlock(VOID)
{
#ifdef WIN32
	::LeaveCriticalSection(&m_Lock);
	return TRUE;
#else
	return !::pthread_mutex_unlock(&m_Lock);
#endif
}

/************************************************************************/

DAutoLock::DAutoLock(DMutex &lock, DWORD timeout /* = 0UL */) :
	m_Lock(lock)
{
	DVerify(m_Lock.Lock(TRUE, timeout));
}

DAutoLock::~DAutoLock()
{
	m_Lock.Unlock();
}

/************************************************************************/
