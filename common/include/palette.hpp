/************************************************************************/
/* File Name   : palette.hpp                                            */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 22nd, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DPalette class declaration                             */
/************************************************************************/

#ifndef __SD_COMMON_PALETTE_HPP__
#define __SD_COMMON_PALETTE_HPP__

/************************************************************************/

#include <common.h>
#include <image.h>

/************************************************************************/

class DPalette {

public:

	DPalette();
	explicit DPalette(PALPTR pal);
	~DPalette();

	BOOL IsValid(VOID) CONST;
	BOOL Attach(PALPTR pal);
	BOOL Detach(VOID);
	COLOR GetColor(INT entry) CONST;
	VOID SetColor(INT entry, COLOR color);
	PALPTR GetPalette(VOID);
	PALCPTR GetPalette(VOID) CONST;

	DPalette &operator = (CONST DPalette &pal);
	DPalette &operator = (PALCPTR pal);

	operator PALPTR ();
	operator PALCPTR () CONST;

#ifdef _WIN32
	operator LPPALETTEENTRY () CONST;
#endif

protected:

	static CLRPTR AllocData(VOID);
	static VOID FreeData(CLRPTR data);

	BOOL	m_Attach;
	CLRPTR	m_Palette;

private:

	DPalette(CONST DPalette &pal);

};

/************************************************************************/

#endif	/* __SD_COMMON_PALETTE_HPP__ */
