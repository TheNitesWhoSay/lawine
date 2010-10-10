/************************************************************************/
/* File Name   : text.hpp                                               */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 5th, 2008                                          */
/* Module      : Common library                                         */
/* Descript    : DText class declaration                                */
/************************************************************************/

#ifndef __SD_COMMON_TEXT_HPP__
#define __SD_COMMON_TEXT_HPP__

/************************************************************************/

#include <common.h>

/************************************************************************/

class DText {

public:

	enum TEXT_FORMAT {
		TF_NONE,
		TF_PURE,
		TF_RTF,
	};

public:

	DText();
	~DText();

	BOOL IsValid(VOID) CONST;
	UINT GetLength(VOID) CONST;
	TEXT_FORMAT GetFormat(VOID) CONST;
	STRCPTR GetData(VOID) CONST;
	STRCPTR GetText(VOID) CONST;

	BOOL Attach(STRCPTR data, TEXT_FORMAT fmt, UINT width = 0U, BOOL warp = FALSE);
	BOOL Detach(VOID);
	BOOL Create(STRCPTR data, TEXT_FORMAT fmt, UINT width = 0U, BOOL warp = FALSE, BOOL pre_alloc = FALSE);
	DText *Copy(VOID) CONST;
	BOOL Release(VOID);

	static STRPTR AllocData(STRCPTR data, UINT len = 0U);
	static VOID FreeData(STRPTR data);

protected:

	static BOOL CheckText(STRCPTR data, TEXT_FORMAT fmt, UINT width, BOOL warp);
	static DText *CreateText(STRCPTR data, TEXT_FORMAT fmt, UINT width, BOOL warp, BOOL pre_alloc);

	BOOL			m_Attach;
	BOOL			m_Warp;
	UINT			m_Width;
	UINT			m_Length;
	TEXT_FORMAT		m_Format;
	STRCPTR			m_Data;

};

/************************************************************************/

#endif	/* __SD_COMMON_TEXT_HPP__ */
