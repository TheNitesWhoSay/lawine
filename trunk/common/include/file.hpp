/************************************************************************/
/* File Name   : file.hpp                                               */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 18th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DFile class declaration                                */
/************************************************************************/

#ifndef __SD_COMMON_FILE_HPP__
#define __SD_COMMON_FILE_HPP__

/************************************************************************/

#include <common.h>
#include <string.hpp>

/************************************************************************/

class DFile {

public:

	enum OPEN_MODE {
		OM_READ			= 0x00000001,
		OM_WRITE		= 0x00000002,
		OM_READWRITE	= OM_READ | OM_WRITE,
		OM_CREATE		= 0x00000004,
		OM_TRUNCATE		= 0x00000008,
	};

#ifdef WIN32
	typedef HANDLE	FILEHANDLE;
#else
	typedef FILE	*FILEHANDLE;
#endif

public:

	DFile();
	~DFile();

	STRCPTR GetName(VOID) CONST;
	BOOL IsOpen(VOID) CONST;
	BOOL Open(STRCPTR name, INT mode = OM_READ);
	BOOL Close(VOID);
	BOOL CreateTempFile(VOID);
	UINT Read(VPTR buf, UINT size);
	UINT ReadLine(STRPTR buf, UINT size = 0U);
	UINT ReadFormat(STRPTR buf, STRCPTR fmt, ...);
	UINT Write(VCPTR buf, UINT size);
	UINT WriteLine(STRCPTR buf, UINT size = 0U);
	UINT WriteFormat(STRCPTR fmt, ...);
	VOID Flush(VOID);
	UINT Position(VOID) CONST;
	UINT Seek(INT offset, SEEK_MODE mode = SM_BEGIN);
	VOID Rewind(VOID);
	UINT GetSize(VOID) CONST;
	BOOL SetSize(UINT size);
	BOOL IsEnd(VOID) CONST;
	BOOL Attach(FILEHANDLE handle, STRCPTR name = NULL);
	FILEHANDLE Detach(VOID);

	static BOOL IsValidHandle(FILEHANDLE handle);
	static UINT GetFullPath(STRCPTR name, STRPTR buf = NULL, UINT buf_size = 0U);
	static BOOL IsExist(STRCPTR name);
	static BOOL IsFile(STRCPTR name);
	static BOOL IsDir(STRCPTR name);
	static BOOL Remove(STRCPTR name);

protected:

	DString		m_Name;
	BOOL		m_Temp;
	FILEHANDLE	m_File;

private:

	DFile(CONST DFile &file);

	DFile &operator = (CONST DFile &file);

};

/************************************************************************/

#endif	/* __SD_COMMON_FILE_HPP__ */
