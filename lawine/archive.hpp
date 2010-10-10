/************************************************************************/
/* File Name   : archive.hpp                                            */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 23rd, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : DArchive class declaration                             */
/************************************************************************/

#ifndef __SD_LAWINE_ARCHIVE_HPP__
#define __SD_LAWINE_ARCHIVE_HPP__

/************************************************************************/

#include <list>
#include "data/mpq.hpp"

/************************************************************************/

class DArchive {

protected:

	struct ARCHIVE {
		DMpq *mpq;
		UINT priority;
	};

	typedef std::list<ARCHIVE>	DArcList;

public:

	DArchive();
	~DArchive();

	DMpq *UseArchive(STRCPTR mpq_name, UINT priority = 0U);
	BOOL CloseArchive(DMpq *mpq);
	BOOL FileExist(STRCPTR file_name);
	HANDLE OpenFile(STRCPTR file_name);
	BOOL CloseFile(HANDLE file);
	UINT GetFileSize(HANDLE file);
	UINT ReadFile(HANDLE file, VPTR data, UINT size);
	UINT SeekFile(HANDLE file, INT offset, SEEK_MODE mode = SM_BEGIN);
	HANDLE OpenHandle(STRCPTR file_name);

protected:

	DMpq *SearchFile(STRCPTR file_name) CONST;
	DMpq *SearchFile(HANDLE file) CONST;

	DArcList	m_ArcList;

};

/************************************************************************/

#endif	/* __SD_LAWINE_ARCHIVE_HPP__ */
