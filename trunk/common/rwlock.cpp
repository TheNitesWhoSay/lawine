/************************************************************************/
/* File Name   : rwlock.cpp                                             */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 18th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DRwLock class implementation                           */
/************************************************************************/

#include <rwlock.hpp>

/************************************************************************/

DRwLock::DRwLock()
#ifdef _WIN32
:	m_RW(FALSE)
#endif
{
#ifdef _WIN32
	::InitializeCriticalSection(&m_RdLock);
	::InitializeCriticalSection(&m_WrLock);
#else
	::pthread_rwlock_init(&m_Lock, NULL);
#endif
}

DRwLock::~DRwLock()
{
#ifdef _WIN32
	::DeleteCriticalSection(&m_WrLock);
	::DeleteCriticalSection(&m_RdLock);
#else
	::pthread_rwlock_destroy(&m_Lock);
#endif
}

BOOL DRwLock::Read(BOOL wait /* = TRUE */, UINT timeout /* = 0U */) CONST
{
#ifdef _WIN32
	::EnterCriticalSection(&m_WrLock);
	m_RW = FALSE;
	return TRUE;
#else
	if (!wait)
		return !::pthread_rwlock_tryrdlock(&m_Lock);

	if (!timeout)
		return !::pthread_rwlock_rdlock(&m_Lock);

	timespec ts;
	ts.tv_sec = timeout;
	ts.tv_nsec = timeout % 1000 * 1000000;

	return !::pthread_rwlock_timedrdlock(&m_Lock, &ts);
#endif
}

BOOL DRwLock::Write(BOOL wait /* = TRUE */, UINT timeout /* = 0U */)
{
#ifdef _WIN32
	::EnterCriticalSection(&m_RdLock);
	::EnterCriticalSection(&m_WrLock);
	m_RW = TRUE;
	return TRUE;
#else
	if (!wait)
		return !::pthread_rwlock_trywrlock(&m_Lock);

	if (!timeout)
		return !::pthread_rwlock_wrlock(&m_Lock);

	timespec ts;
	ts.tv_sec = timeout;
	ts.tv_nsec = timeout % 1000 * 1000000;

	return !::pthread_rwlock_timedwrlock(&m_Lock, &ts);
#endif
}

BOOL DRwLock::Unlock(VOID) CONST
{
#ifdef _WIN32
	BOOL rw = m_RW;
	m_RW = FALSE;
	::LeaveCriticalSection(&m_WrLock);

	if (m_RW)
		::LeaveCriticalSection(&m_RdLock);

	return TRUE;
#else
	return !::pthread_rwlock_unlock(&m_Lock);
#endif
}

/************************************************************************/
