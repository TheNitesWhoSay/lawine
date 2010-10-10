/************************************************************************/
/* File Name   : text.cpp                                               */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 5th, 2008                                          */
/* Module      : Common library                                         */
/* Descript    : DText class implementation                            */
/************************************************************************/

#include <text.hpp>

/************************************************************************/

DText::DText() :
	m_Attach(FALSE),
	m_Warp(FALSE),
	m_Width(0U),
	m_Length(0U),
	m_Format(TF_NONE),
	m_Data(NULL)
{

}

DText::~DText()
{
	if (!m_Attach)
		FreeData(const_cast<STRPTR>(m_Data));
}

/************************************************************************/

BOOL DText::IsValid(VOID) CONST
{
	return m_Data != NULL;
}

UINT DText::GetLength(VOID) CONST
{
	return m_Length;
}

DText::TEXT_FORMAT DText::GetFormat(VOID) CONST
{
	return m_Format;
}

STRCPTR DText::GetData(VOID) CONST
{
	return m_Data;
}

STRCPTR DText::GetText(VOID) CONST
{
	if (!m_Data)
		return NULL;

	// TODO:
	return NULL;
}

BOOL DText::Attach(STRCPTR data, TEXT_FORMAT fmt, UINT width /* = 0U */, BOOL warp /* = FALSE */)
{
	if (m_Data)
		return FALSE;

	if (!CheckText(data, fmt, width, warp))
		return FALSE;

	m_Attach = TRUE;
	m_Warp = warp;
	m_Width = width;
	m_Format = fmt;
	m_Data = data;
	m_Length = DStrLen(m_Data);
	return TRUE;
}

BOOL DText::Detach(VOID)
{
	if (!m_Attach)
		return FALSE;

	m_Attach = FALSE;
	m_Warp = FALSE;
	m_Width = 0U;
	m_Format = TF_NONE;
	m_Data = NULL;
	m_Length = 0U;
	return TRUE;
}

BOOL DText::Create(STRCPTR data, TEXT_FORMAT fmt, UINT width /* = 0U */, BOOL warp /* = FALSE */, BOOL pre_alloc /* = FALSE */)
{
	if (m_Data)
		return FALSE;

	if (!CheckText(data, fmt, width, warp))
		return FALSE;

	if (pre_alloc) {
		m_Data = data;
	} else {
		m_Data = AllocData(data);
		if (!m_Data)
			return FALSE;
	}

	m_Attach = FALSE;
	m_Warp = warp;
	m_Width = width;
	m_Format = fmt;
	m_Length = DStrLen(m_Data);
	return TRUE;
}

DText *DText::Copy(VOID) CONST
{
	if (!m_Data)
		return NULL;

	return CreateText(m_Data, m_Format, m_Width, m_Warp, FALSE);
}

BOOL DText::Release(VOID)
{
	if (m_Attach)
		return FALSE;

	FreeData(const_cast<STRPTR>(m_Data));
	m_Data = NULL;
	m_Length = 0U;
	m_Format = TF_NONE;
	m_Width = 0U;
	m_Warp = FALSE;
	return TRUE;
}

STRPTR DText::AllocData(STRCPTR data, UINT len /* = 0U */)
{
	if (!data) {
		if (len)
			return new CHAR[len + 1];
		return NULL;
	}

	if (!len)
		len = DStrLen(data);

	STRPTR buf = new CHAR[len + 1];
	DStrCpyN(buf, data, len);
	buf[len] = '\0';
	return buf;
}

VOID DText::FreeData(STRPTR data)
{
	delete [] data;
}

BOOL DText::CheckText(STRCPTR data, TEXT_FORMAT fmt, UINT width, BOOL warp)
{
	if (!data)
		return FALSE;

	switch (fmt) {
	case TF_PURE:
	case TF_RTF:
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

DText *DText::CreateText(STRCPTR data, TEXT_FORMAT fmt, UINT width, BOOL warp, BOOL pre_alloc)
{
	DText *text = new DText;
	if (!text->Create(data, fmt, width, warp, pre_alloc)) {
		delete text;
		return NULL;
	}

	return text;
}

/************************************************************************/
