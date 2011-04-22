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

UINT DChk::GetSectionSize(DWORD fourcc, SECTION_TYPE type) CONST
{
	if (!m_File)
		return 0U;

	if (!fourcc)
		return 0U;

	switch (type) {
	case ST_OVERRIDE:
	case ST_DUPLICATE:
	case ST_LASTONE:
		break;
	default:
		return 0U;
	}

	DSectionMap::const_iterator map_it = m_SectionTable.find(fourcc);
	if (map_it == m_SectionTable.end())
		return 0U;

	UINT size = 0U;
	DSectionList::const_iterator it = map_it->second.begin();
	for (; it != map_it->second.end(); ++it) {
		if (!it->size)
			continue;
		switch (type) {
		case ST_OVERRIDE:
			if (size < it->size)
				size = it->size;
			break;
		case ST_DUPLICATE:
			size += it->size;
			break;
		case ST_LASTONE:
			size = it->size;
			break;
		}
	}

	return size;
}

BOOL DChk::GetSectionData(DWORD fourcc, SECTION_TYPE type, VPTR buf, UINT buf_size) CONST
{
	if (!m_File)
		return FALSE;

	if (!fourcc || !buf || !buf_size)
		return FALSE;

	switch (type) {
	case ST_OVERRIDE:
	case ST_DUPLICATE:
	case ST_LASTONE:
		break;
	default:
		return FALSE;
	}

	DSectionMap::const_iterator map_it = m_SectionTable.find(fourcc);
	if (map_it == m_SectionTable.end() || !map_it->second.size())
		return FALSE;

	BUFPTR copy_ptr = static_cast<BUFPTR>(buf);
	DSectionList::const_iterator it = map_it->second.begin();
	for (; it != map_it->second.end(); ++it) {
		if (!it->size)
			continue;
		UINT copy_size = (buf_size > it->size) ? it->size : buf_size;
		if (::g_Archive.SeekFile(m_File, it->offset) == ERROR_POS)
			return FALSE;
		if (::g_Archive.ReadFile(m_File, copy_ptr, copy_size) != copy_size)
			return FALSE;
		if (type != ST_DUPLICATE)
			continue;
		copy_ptr += copy_size;
		buf_size -= copy_size;
		if (!buf_size)
			break;
	}

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

		offset += header.size;
		if (offset >= size)
			return TRUE;

		m_SectionTable[header.fourcc].push_back(info);

		if (::g_Archive.SeekFile(file, header.size, SM_CURRENT) == ERROR_POS)
			break;
	}

	return FALSE;
}

/************************************************************************/
