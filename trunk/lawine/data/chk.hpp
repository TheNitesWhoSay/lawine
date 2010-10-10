/************************************************************************/
/* File Name   : chk.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 20th, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : DChk class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_CHK_HPP__
#define __SD_LAWINE_DATA_CHK_HPP__

/************************************************************************/

#include <common.h>
#include <map>

/************************************************************************/

class DChk {

protected:

	struct SECTIONHEADER {
		DWORD	fourcc;
		DWORD	size;
	};

	struct SECTIONINFO {
		UINT	size;
		DWORD	offset;
	};

	typedef std::map<DWORD, SECTIONINFO>	DSectionMap;

public:

	DChk();
	~DChk();

	BOOL Load(STRCPTR name);
	VOID Clear(VOID);
	UINT GetSectionSize(DWORD fourcc);
	BOOL GetSectionData(DWORD fourcc, VPTR buf, UINT buf_size);

protected:

	BOOL Analysis(HANDLE file, UINT size);

	HANDLE		m_File;
	DSectionMap	m_SectionTable;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_CHK_HPP__ */
