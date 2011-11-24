/************************************************************************/
/* File Name   : tbl.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 7th, 2011                                          */
/* Module      : Lawine library                                         */
/* Descript    : DTbl class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_TBL_HPP__
#define __SD_LAWINE_DATA_TBL_HPP__

/************************************************************************/

#include <common.h>
#include <string.hpp>
#include <vector>

/************************************************************************/

class DTbl {

public:

	DTbl();
	~DTbl();

	BOOL Load(STRCPTR name);
	VOID Clear(VOID);

	INT GetCount(VOID) CONST;
	STRCPTR GetString(INT index) CONST;

protected:

	typedef std::vector<DString>	DStrTable;

	BOOL Analysis(HANDLE file);

	DStrTable	m_Table;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_TBL_HPP__ */
