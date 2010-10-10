/************************************************************************/
/* File Name   : debug.h                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 21st, 2009                                         */
/* Module      : Common library                                         */
/* Descript    : Common debug tools definition                          */
/************************************************************************/

#ifndef __SD_COMMON_DEBUG_H__
#define __SD_COMMON_DEBUG_H__

/************************************************************************/

#include <common.h>

/************************************************************************/

#ifndef NDEBUG

struct _DEBUG_TIMER {
#ifdef WIN32
	QWORD begin;
	QWORD time;
#else
#endif
	DWORD cnt;
	STRCPTR msg;
	STRCPTR file;
	INT line;
	STRCPTR func;
};

#endif

/************************************************************************/

#ifdef NDEBUG

#define InitializeDebugTimer()
#define OutputDebugTimerResult()

#define DEBUG_TIMER_DECLEAR_VAR(v)
#define DEBUG_TIMER_BEGIN_VAR(v, m)
#define DEBUG_TIMER_END_VAR(v)

#define DEBUG_TIMER_DECLEAR()
#define DEBUG_TIMER_BEGIN(m)
#define DEBUG_TIMER_END()

#define DEBUG_TIMER_AUTO(m)

#else

#ifdef WIN32

CAPI extern HANDLE _debug_timer_current_thread;
CAPI extern FILETIME _debug_timer_dummy;
CAPI extern QWORD _debug_timer_tmp;

CAPI extern VOID InitializeDebugTimer(VOID);
CAPI extern VOID OutputDebugTimerResult(VOID);

#define _DEBUG_TIMER_DECLEAR(t)	\
	static struct _DEBUG_TIMER *t;

#define _DEBUG_TIMER_BEGIN(t)	\
	do { \
		GetThreadTimes( \
			_debug_timer_current_thread, \
			&_debug_timer_dummy, \
			&_debug_timer_dummy, \
			&_debug_timer_dummy, \
			(LPFILETIME)&(t)->begin \
		); \
		(t)->cnt++; \
	} while (FALSE)

#define _DEBUG_TIMER_END(t)		\
	do { \
		GetThreadTimes( \
			_debug_timer_current_thread, \
			&_debug_timer_dummy, \
			&_debug_timer_dummy, \
			&_debug_timer_dummy, \
			(LPFILETIME)&_debug_timer_tmp \
		); \
		(t)->time += _debug_timer_tmp - (t)->begin; \
	} while(FALSE)

CAPI struct _DEBUG_TIMER * __fastcall _register_debug_timer(STRCPTR msg, STRCPTR file, INT line, STRCPTR func);

#else

	// TODO:

#define InitializeDebugTimer()
#define OutputDebugTimerResult()

#define _DEBUG_TIMER_DECLEAR(t)
#define _DEBUG_TIMER_BEGIN(t)
#define _DEBUG_TIMER_END(t)
#define _register_debug_timer(m, f, l, n)	NULL

#endif

#ifdef __cplusplus

#define DEBUG_TIMER_DECLEAR_VAR(v)

#define DEBUG_TIMER_BEGIN_VAR(v, m)	\
	_DEBUG_TIMER_DECLEAR(_debug_timer_##v); \
	do { \
		_debug_timer_##v = _register_debug_timer((m), __FILE__, __LINE__, __FUNCTION__); \
		_DEBUG_TIMER_BEGIN(_debug_timer_##v); \
	} while (FALSE)

#define DEBUG_TIMER_END_VAR(v)	_DEBUG_TIMER_END(_debug_timer_##v)

#define DEBUG_TIMER_AUTO(m)		\
	static _DEBUG_TIMER *_debug_timer__auto_ = _register_debug_timer((m), __FILE__, __LINE__, __FUNCTION__); \
	_DDebugTimerAuto _debug_timer_auto_obj(_debug_timer__auto_)

class _DDebugTimerAuto {

public:

	_DDebugTimerAuto(_DEBUG_TIMER *timer)
	{
		_timer = timer;
		_DEBUG_TIMER_BEGIN(_timer);
	}

	~_DDebugTimerAuto()
	{
		_DEBUG_TIMER_END(_timer);
	}

private:

	_DEBUG_TIMER *_timer;

};

#else

#define DEBUG_TIMER_DECLEAR_VAR(v)	\
	_DEBUG_TIMER_DECLEAR(_debug_timer_##v)

#define DEBUG_TIMER_BEGIN_VAR(v, m)	\
	do { \
		_debug_timer_##v = _register_debug_timer((m), __FILE__, __LINE__, __FUNCTION__); \
		_DEBUG_TIMER_BEGIN(_debug_timer_##v); \
	} while (FALSE)

#define DEBUG_TIMER_END_VAR(v)	_DEBUG_TIMER_END(_debug_timer_##v)

#define DEBUG_TIMER_AUTO(m)

#endif

#define DEBUG_TIMER_DECLEAR()	DEBUG_TIMER_DECLEAR_VAR(_default_)

#define DEBUG_TIMER_BEGIN(m)	DEBUG_TIMER_BEGIN_VAR(_default_, (m))

#define DEBUG_TIMER_END()		DEBUG_TIMER_END_VAR(_default_)

#endif

/************************************************************************/

#endif	/* __SD_COMMON_DEBUG_H__ */
