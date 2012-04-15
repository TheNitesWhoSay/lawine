/************************************************************************/
/* File Name   : debug.c                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 21st, 2009                                         */
/* Module      : Common library                                         */
/* Descript    : Common debug tools implementation                      */
/************************************************************************/

#include <debug.h>

#ifndef NDEBUG

/************************************************************************/

#define MAX_DEBUG_TIMER		100

/************************************************************************/

static struct _DEBUG_TIMER s_DebugTimerList[MAX_DEBUG_TIMER];
static INT s_CurDebugTimer = 0;

#ifdef _WIN32

HANDLE _debug_timer_current_thread = NULL;
FILETIME _debug_timer_dummy;
QWORD _debug_timer_tmp;

#endif

/************************************************************************/

#ifdef _WIN32

struct _DEBUG_TIMER * __fastcall _register_debug_timer(STRCPTR msg, STRCPTR file, INT line, STRCPTR func)
{
	struct _DEBUG_TIMER *p = s_DebugTimerList + s_CurDebugTimer;
	p->msg = msg;
	p->file = file;
	p->line = line;
	p->func = func;
	s_CurDebugTimer++;
	return p;
}

VOID InitializeDebugTimer(VOID)
{
	if (!_debug_timer_current_thread)
		_debug_timer_current_thread = GetCurrentThread();

	DVarClr(s_DebugTimerList);
}

VOID OutputDebugTimerResult(VOID)
{
	INT i;
	CHAR s[1024];
	struct _DEBUG_TIMER *p;

	for (i = 0; i < s_CurDebugTimer; i++) {

		p = s_DebugTimerList + i;

		if (!p->cnt)
			continue;

		DSprintf(
			s,
			sizeof(s),
			"[%.3fms] %d %s %d %s : %s",
			(LONGLONG)p->time / 10000.0,
			p->cnt,
			p->file,
			p->line,
			p->func,
			p->msg
		);

		OutputDebugString(s);
	}

	CloseHandle(_debug_timer_current_thread);
	_debug_timer_current_thread = NULL;
}

#endif

/************************************************************************/

#endif
