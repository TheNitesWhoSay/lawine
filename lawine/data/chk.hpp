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
#include <list>
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

	typedef std::list<SECTIONINFO>			DSectionList;
	typedef std::map<DWORD, DSectionList>	DSectionMap;

public:

	enum SECTION_TYPE {
		ST_OVERRIDE,
		ST_DUPLICATE,
		ST_LASTONE,
	};

	DChk();
	~DChk();

	BOOL Load(STRCPTR name);
	VOID Clear(VOID);

	UINT GetSectionSize(DWORD fourcc, SECTION_TYPE type) CONST;
	BOOL GetSectionData(DWORD fourcc, SECTION_TYPE type, VPTR buf, UINT buf_size) CONST;

protected:

	BOOL Analysis(HANDLE file, UINT size);

	HANDLE		m_File;
	DSectionMap	m_SectionTable;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_CHK_HPP__ */
