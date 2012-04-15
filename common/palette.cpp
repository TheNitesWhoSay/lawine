/************************************************************************/
/* File Name   : palette.cpp                                            */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 23rd, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DPalette class implementation                          */
/************************************************************************/

#include <palette.hpp>

/************************************************************************/

DPalette::DPalette() :
	m_Attach(FALSE),
	m_Palette(AllocData())
{

}

DPalette::DPalette(PALPTR pal) :
	m_Attach(FALSE),
	m_Palette(NULL)
{
	Attach(pal);
}

DPalette::~DPalette()
{
	if (!m_Attach)
		FreeData(m_Palette);
}

/************************************************************************/

BOOL DPalette::IsValid(VOID) CONST
{
	return m_Palette != NULL;
}

BOOL DPalette::Attach(PALPTR pal)
{
	if (m_Palette)
		return FALSE;

	if (!pal)
		return FALSE;

	m_Attach = TRUE;
	m_Palette = pal;
	return TRUE;
}

BOOL DPalette::Detach(VOID)
{
	if (!m_Attach)
		return FALSE;

	m_Palette = NULL;
	m_Attach = FALSE;
	return TRUE;
}

COLOR DPalette::GetColor(INT entry) CONST
{
	DAssert(m_Palette);
	DAssert(DBetween(entry, 0, D_COLOR_NUM));
	return m_Palette[entry];
}

VOID DPalette::SetColor(INT entry, COLOR color)
{
	DAssert(m_Palette);
	DAssert(DBetween(entry, 0, D_COLOR_NUM));
	m_Palette[entry] = color;
}

PALPTR DPalette::GetPalette(VOID)
{
	return m_Palette;
}

PALCPTR DPalette::GetPalette(VOID) CONST
{
	return m_Palette;
}

DPalette &DPalette::operator = (CONST DPalette &pal)
{
	DAssert(m_Palette && pal.m_Palette);
	DMemCpy(m_Palette, pal.m_Palette, D_COLOR_NUM * sizeof(COLOR));
	return *this;
}

DPalette &DPalette::operator = (PALCPTR pal)
{
	DAssert(m_Palette && pal);
	DMemCpy(m_Palette, pal, D_COLOR_NUM * sizeof(COLOR));
	return *this;
}

DPalette::operator PALPTR ()
{
	return m_Palette;
}

DPalette::operator PALCPTR () CONST
{
	return m_Palette;
}

#ifdef _WIN32

DPalette::operator LPPALETTEENTRY () CONST
{
	return reinterpret_cast<LPPALETTEENTRY>(const_cast<CLRPTR>(m_Palette));
}

#endif

/************************************************************************/

CLRPTR DPalette::AllocData(VOID)
{
	return new COLOR[D_COLOR_NUM];
}

VOID DPalette::FreeData(CLRPTR data)
{
	delete [] data;
}

/************************************************************************/

DPalette::DPalette(CONST DPalette &pal)
{
	DAssert(FALSE);
}

/************************************************************************/
