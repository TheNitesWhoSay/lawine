/************************************************************************/
/* File Name   : chk.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 20th, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : DChk class implementation                              */
/************************************************************************/

#include "chk.hpp"
#include "../global.hpp"

/************************************************************************/

DChk::DChk() : m_File(NULL)
{

}

DChk::~DChk()
{

}

/************************************************************************/

BOOL DChk::Load(STRCPTR name)
{
	if (m_File)
		return FALSE;

	if (!name)
		return FALSE;

	HANDLE file = ::g_Archive.OpenFile(name);
	if (!file)
		return FALSE;

	UINT size = ::g_Archive.GetFileSize(file);
	if (size <= sizeof(SECTIONHEADER)) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	if (!Analysis(file, size) || !m_SectionTable.size()) {
		Clear();
		return FALSE;
	}

	m_File = file;
	return TRUE;
}

VOID DChk::Clear(VOID)
{
	m_SectionTable.clear();

	if (m_File) {
		::g_Archive.CloseFile(m_File);
		m_File = NULL;
	}
}

UINT DChk::GetSectionSize(DWORD fourcc)
{
	if (!m_SectionTable.count(fourcc))
		return 0U;

	return m_SectionTable[fourcc].size;
}

BOOL DChk::GetSectionData(DWORD fourcc, VPTR buf, UINT buf_size)
{
	if (!buf || !buf_size)
		return FALSE;

	if (!m_File)
		return FALSE;

	if (!m_SectionTable.count(fourcc))
		return FALSE;

	SECTIONINFO &info = m_SectionTable[fourcc];

	if (buf_size > info.size)
		buf_size = info.size;

	if (::g_Archive.SeekFile(m_File, info.offset) == ERROR_POS)
		return FALSE;

	if (::g_Archive.ReadFile(m_File, buf, buf_size) != buf_size)
		return FALSE;

	return TRUE;
}

/************************************************************************/

BOOL DChk::Analysis(HANDLE file, UINT size)
{
	DAssert(file);

	UINT offset = 0U;
	UINT rd_size = sizeof(SECTIONHEADER);
	SECTIONHEADER header;
	SECTIONINFO info;

	while (::g_Archive.ReadFile(file, &header, rd_size) == rd_size) {

		offset += rd_size;

		info.size = header.size;
		info.offset = offset;
		m_SectionTable[header.fourcc] = info;

		if (::g_Archive.SeekFile(file, header.size, SM_CURRENT) == ERROR_POS)
			break;

		offset += header.size;
		if (offset >= size)
			return TRUE;
	}

	return FALSE;
}

/************************************************************************/
