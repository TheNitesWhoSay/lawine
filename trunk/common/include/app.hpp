/************************************************************************/
/* File Name   : app.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Nov 27th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DApp class declaration                                 */
/************************************************************************/

#ifndef __SD_COMMON_APP_HPP__
#define __SD_COMMON_APP_HPP__

/************************************************************************/

#include <common.h>
#include <list>
#include <algorithm>

/************************************************************************/

class DApp {

public:

	struct APPLICATION {
		STRCPTR name;
#ifdef _WIN32
		HINSTANCE hinst;
		LPSTR cmd;
		INT show;
#else
		INT argc;
		STRPTR *argv;
#endif
	};

public:

	DApp();
	~DApp();

	BOOL Initialize();
	VOID Run(VOID);
	VOID Exit(VOID);
	INT Return(VOID);

	virtual BOOL Prepare(APPLICATION &app, POINT &pos, SIZE &size) = 0;
	virtual BOOL Start(VOID) = 0;
	virtual BOOL Loop(QWORD tick) = 0;
	virtual VOID End(VOID) = 0;

	virtual VOID OnLButtonDown(CONST POINT &pos) = 0;
	virtual VOID OnLButtonUp(CONST POINT &pos) = 0;
	virtual VOID OnRButtonDown(CONST POINT &pos) = 0;
	virtual VOID OnRButtonUp(CONST POINT &pos) = 0;
	virtual VOID OnChar(UINT ch) = 0;
	virtual VOID OnAltEnter(VOID) = 0;

protected:

	BOOL Check(VOID) CONST;

protected:

	INT				m_RetCode;
	APPLICATION		m_Core;
	POINT			m_Pos;
	SIZE			m_Size;
#ifdef WIN32
	HWND			m_Wnd;
	LARGE_INTEGER	m_Freq;
	LARGE_INTEGER	m_StartTime;
#endif

};

/************************************************************************/

#endif	/* __SD_COMMON_APP_HPP__ */
