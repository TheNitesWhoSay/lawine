/************************************************************************/
/* File Name   : fnt.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 8th, 2009                                          */
/* Module      : Lawine library                                         */
/* Descript    : DFnt class implementation                              */
/************************************************************************/

#include "fnt.hpp"
#include "../global.hpp"
#include "../misc/fontdec.h"

/************************************************************************/

CONST DWORD IDENTIFIER = 'TNOF';
CONST STRCPTR FONT_GID_PATH = "font\\font.gid";
CONST STRCPTR FONT_CCD_PATH = "font\\font.ccd";

/************************************************************************/

BUFPTR DFnt::s_Gid;

/************************************************************************/

DFnt::DFnt() :
	m_CharBuf(NULL),
	m_DataBuf(NULL),
	m_DataSize(0U),
	m_CurChar('\0'),
	m_CharHeader(NULL)
{
	
}

DFnt::~DFnt()
{
	Clear();
}

/************************************************************************/

BOOL DFnt::GetMaxSize(SIZE &size) CONST
{
	if (!m_Header) {
		size.cx = 0;
		size.cy = 0;
		return FALSE;
	}

	size.cx = m_Header->width;
	size.cy = m_Header->height;
	return TRUE;
}

BOOL DFnt::Load(STRCPTR name)
{
	if (m_DataBuf)
		return FALSE;

	UINT size;
	HANDLE file = ::g_Archive.OpenFile(name);
	BUFPTR data = LoadFile(file, size);
	if (!data)
		return FALSE;

	HEADER *head = reinterpret_cast<HEADER *>(data);
	if ((head->id == IDENTIFIER) || decrypt_font(data, size)) {
		if (!Load(data, size)) {
			delete [] data;
			return FALSE;
		}
	}

	m_DataBuf = data;
	m_DataSize = size;
	return TRUE;
}

VOID DFnt::Clear(VOID)
{
	delete [] m_CharBuf;
	m_CharBuf = NULL;
	m_CurChar = '\0';
	m_CharHeader = NULL;

	delete [] m_DataBuf;
	m_DataBuf = NULL;
	m_DataSize = 0U;
	m_Header = NULL;

	m_Template = NULL;
}

BOOL DFnt::SetTemplate(IMGCPTR temp)
{
	if (!temp || !temp->data)
		return FALSE;

	if (temp->size.cx < STYLE_NUM)
		return FALSE;

	m_Template = temp;
	return TRUE;
}

BOOL DFnt::GetCharSize(BYTE ch, SIZE &size) CONST
{
	if (ch == m_CurChar && m_CharHeader) {
		size.cx = m_CharHeader->width;
		size.cy = m_CharHeader->height;
		return TRUE;
	}

	CONST CHARHEADER *head = GetCharHeader(ch);
	if (head) {
		size.cx = head->width;
		size.cy = head->height;
		return TRUE;
	}

	// 空格特殊处理
	if (m_Header) {
		size.cx = m_Header->width / 2;
		size.cy = m_Header->height;
		return TRUE;
	}

	return FALSE;
}

BOOL DFnt::GetChar(DImage &img, BYTE ch, INT style)
{
	if (!img.IsValid() || !DBetween(style, 0, STYLE_NUM))
		return FALSE;

	if (!m_Header || !m_Template)
		return FALSE;

	if (img.GetWidth() < m_Header->width || img.GetHeight() < m_Header->height)
		return FALSE;

	if (!Decode(ch))
		return FALSE;

	Blit(img.GetData(), img.GetPitch(), style);
	return TRUE;
}

BOOL DFnt::Initialize(VOID)
{
	if (s_Gid)
		return TRUE;

	UINT gid_size;
	UINT ccd_size;

	HANDLE gid = ::g_Archive.OpenFile(FONT_GID_PATH);
	s_Gid = LoadFile(gid, gid_size, FONT_GID_SIZE);
	::g_Archive.CloseFile(gid);

	if (!s_Gid)
		return FALSE;

	HANDLE ccd = ::g_Archive.OpenFile(FONT_CCD_PATH);
	BUFPTR ccd_data = LoadFile(ccd, ccd_size);
	::g_Archive.CloseFile(ccd);

	BOOL ret = FALSE;
	if (ccd)
		ret = init_font_decrypt(s_Gid, ccd_data, ccd_size);

	delete [] ccd_data;
	if (ret)
		return TRUE;

	delete [] s_Gid;
	s_Gid = NULL;
	return FALSE;
}

VOID DFnt::Exit(VOID)
{
	exit_font_decrypt();

	delete [] s_Gid;
	s_Gid = NULL;
}

/************************************************************************/

BOOL DFnt::Load(BUFCPTR data, UINT size)
{
	DAssert(data && size);

	CONST HEADER *head = reinterpret_cast<CONST HEADER *>(data);
	CONST DWORD *char_off = reinterpret_cast<CONST DWORD *>(data + sizeof(HEADER));
	if (!CheckHeader(head, char_off, size))
		return FALSE;

	m_Header = head;

	UINT buf_size = m_Header->width * m_Header->height;
	m_CharBuf = new BYTE[buf_size];
	DMemClr(m_CharBuf, buf_size);
	return TRUE;
}

BOOL DFnt::Decode(BYTE ch)
{
	DAssert(m_CharBuf);

	if (ch == m_CurChar)
		return TRUE;

	CONST CHARHEADER *head = GetCharHeader(ch);
	UINT size = head ? (head->width * head->height) : (m_Header->width / 2 * m_Header->height);
	DMemClr(m_CharBuf, size);

	if (head) {

		BUFCPTR p = reinterpret_cast<BUFCPTR>(head + 1);
		BUFPTR q = m_CharBuf;

		for (UINT i = 0; i < size; i++, p++, q++) {
			INT skip = *p >> 3;
			q += skip;
			*q = *p & 0x07;
			i += skip;
		}
	}

	m_CharHeader = head;
	m_CurChar = ch;
	return TRUE;
}

CONST DFnt::CHARHEADER *DFnt::GetCharHeader(BYTE ch) CONST
{
	if (ch < m_Header->begin || ch > m_Header->end)
		return NULL;

	CONST DWORD *char_off = reinterpret_cast<CONST DWORD *>(m_DataBuf + sizeof(HEADER));
	INT entry = ch - m_Header->begin;
	DWORD offset = char_off[entry];
	if (!entry)
		return NULL;

	return reinterpret_cast<CONST CHARHEADER *>(m_DataBuf + offset);
}

VOID DFnt::Blit(BUFPTR buf, UINT pitch, INT style)
{
	DAssert(buf && pitch && m_Template && m_CharBuf && DBetween(style, 0, STYLE_NUM));

	if (!m_CharHeader) {

		for (INT j = 0; j < m_Header->height; j++) {
			DMemClr(buf, m_Header->width / 2);
			buf += pitch;
		}

		return;
	}

	BUFCPTR p = m_CharBuf;
	BUFCPTR temp = m_Template->data;
	DAssert(temp);
	temp += style << 3;
	buf += m_CharHeader->hor_offset + m_CharHeader->ver_offset * pitch;

	for (INT j = 0; j < m_CharHeader->height; j++) {
		for (INT i = 0; i < m_CharHeader->width; i++, p++) {
			if (*p)
				buf[i] = temp[*p];
		}
		buf += pitch;
	}
}

BOOL DFnt::CheckHeader(CONST HEADER *head, CONST DWORD *char_off, UINT size)
{
	DAssert(head && char_off && size);

	if (size <= sizeof(HEADER))
		return FALSE;

	if (head->id != IDENTIFIER)
		return FALSE;

	if (head->begin >= ' ' && head->begin > head->end)
		return FALSE;

	if (!head->width || !head->height)
		return FALSE;

	INT char_num = head->end - head->begin + 1;
	if (size <= sizeof(HEADER) + char_num * sizeof(DWORD))
		return FALSE;

	return TRUE;
}

BUFPTR DFnt::LoadFile(HANDLE file, UINT &rd_size, UINT size /* = 0U */)
{
	if (!file)
		return NULL;

	rd_size = ::g_Archive.GetFileSize(file);
	if (!rd_size) {
		::g_Archive.CloseFile(file);
		return NULL;
	}

	if (size && rd_size != size)
		return NULL;

	BUFPTR data = new BYTE[rd_size];
	if (::g_Archive.ReadFile(file, data, rd_size) != rd_size) {
		delete [] data;
		return NULL;
	}

	return data;
}

/************************************************************************/
