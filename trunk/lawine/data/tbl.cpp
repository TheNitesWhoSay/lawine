/************************************************************************/
/* File Name   : tbl.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 7th, 2011                                          */
/* Module      : Lawine library                                         */
/* Descript    : DTbl class implementation                              */
/************************************************************************/

#include "tbl.hpp"
#include "../global.hpp"

/************************************************************************/

DTbl::DTbl()
{

}

DTbl::~DTbl()
{

}

/************************************************************************/

BOOL DTbl::Load(STRCPTR name)
{
	if (m_Table.size())
		return FALSE;

	if (!name)
		return FALSE;

	HANDLE file = ::g_Archive.OpenFile(name);
	if (!file)
		return FALSE;

	BOOL ret = Analysis(file);
	::g_Archive.CloseFile(file);

	return ret;
}

VOID DTbl::Clear(VOID)
{
	m_Table.clear();
}

INT DTbl::GetCount(VOID) CONST
{
	return m_Table.size();
}

STRCPTR DTbl::GetString(INT index) CONST
{
	if (index < 0 || index >= static_cast<INT>(m_Table.size()))
		return NULL;

	return m_Table.at(index);
}

/************************************************************************/

BOOL DTbl::Analysis(HANDLE file)
{
	WORD str_num;
	if (::g_Archive.ReadFile(file, &str_num, sizeof(str_num)) != sizeof(str_num))
		return FALSE;

	INT offset = sizeof(str_num);

	for (UINT i = 0; i < str_num; i++, offset += sizeof(WORD)) {

		if (::g_Archive.SeekFile(file, offset) == ERROR_POS)
			break;

		WORD str_off;
		if (::g_Archive.ReadFile(file, &str_off, sizeof(str_off)) != sizeof(str_off))
			break;
		if (::g_Archive.SeekFile(file, str_off) == ERROR_POS)
			break;

		// TODO: 需要调查SC支持的TBL文件的最长字符串的字符数
		CHAR rd_buf[1024];
		DString string;
		do {
			if (!::g_Archive.ReadFile(file, rd_buf, 1023))
				break;
			rd_buf[1023] = '\0';
			string.Append(rd_buf);
		} while (DStrLen(rd_buf) >= sizeof(rd_buf) - 1);

		m_Table.push_back(string);
	}

	return TRUE;
}

/************************************************************************/
