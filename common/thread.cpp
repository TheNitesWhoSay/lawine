/************************************************************************/
/* File Name   : thread.cpp                                             */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 15th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DThread class implementation                           */
/************************************************************************/

#include <thread.hpp>

/************************************************************************/

struct PARAMETER {
	DThread *thread;
	VPTR param;
};

/************************************************************************/

static INT ThreadRoutine(CONST PARAMETER *param)
{
	if (!param || !param->thread)
		return DThread::RV_PARAM;

	return param->thread->Process(param->param) ? DThread::RV_SUCCESS : DThread::RV_ERROR;
}

/************************************************************************/

DThread::DThread() :
	m_Thread(NULL)
{

}

INT DThread::GetPriority(VOID) CONST
{
#ifdef WIN32
	return ::GetThreadPriority(m_Thread);
#else
	return 0;
#endif
}

VOID DThread::SetPriority(INT priority)
{
#ifdef WIN32
	::SetThreadPriority(m_Thread, priority);
#endif
}

BOOL DThread::Run(VPTR param)
{
	PARAMETER tmp_parm = { this, param };

#ifdef WIN32
	m_Thread = ::CreateThread(NULL, 0, reinterpret_cast<PTHREAD_START_ROUTINE>(&::ThreadRoutine), &tmp_parm, 0, NULL);
	if (!m_Thread)
		return FALSE;
#else
	if (!::pthread_create(&m_Thread, NULL, reinterpret_cast<START_ROUTINE *>(&::ThreadRoutine), &tmp_parm))
		return FALSE;
#endif

	return TRUE;
}

BOOL DThread::Terminate(VOID)
{
	// TODO:
	DAssert(FALSE);
	return FALSE;
}

VOID DThread::Wait(VOID)
{
#ifdef WIN32
	::WaitForSingleObject(m_Thread, INFINITE);
#else
	::pthread_join(m_Thread, NULL);
#endif
}

INT DThread::GetReturn(VOID)
{
	INT ret = RV_UNKNOWN;

#ifdef WIN32
	DWORD tmp;
	::WaitForSingleObject(m_Thread, INFINITE);
	if (::GetExitCodeThread(m_Thread, &tmp))
		ret = tmp;
#else
	::pthread_join(m_Thread, static_cast<VPTR *>&ret);
#endif

	return ret;
}

#if 0
BOOL DThread::GetReturn(INT *ret /* = NULL */)
{
#ifdef WIN32
	DWORD tmp;
	if (!::GetExitCodeThread(m_Thread, &tmp))
		return FALSE;
	if (tmp == STILL_ACTIVE)
		return FALSE;
	if (ret)
		*ret = tmp;
#else
	VPTR tmp;
	if (!::pthread_join(m_Thread, &tmp))
		return FALSE;
	if (ret)
		ret = reinterpret_cast<INT>(tmp);
#endif

	return TRUE;
}
#endif

/************************************************************************/
