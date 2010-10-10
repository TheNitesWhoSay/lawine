/************************************************************************/
/* File Name   : file.cpp                                               */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 18th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DFile class implementation                             */
/************************************************************************/

#include <file.hpp>

/************************************************************************/

#ifdef WIN32
CONST HANDLE INVALID_FILE = INVALID_HANDLE_VALUE;
#else
CONST FILE *INVALID_FILE = NULL;
#endif

/************************************************************************/

DFile::DFile() :
	m_File(INVALID_FILE),
	m_Temp(FALSE)
{

}

DFile::~DFile()
{
	Close();
}

STRCPTR DFile::GetName(VOID) CONST
{
	return m_Name;
}

BOOL DFile::IsOpen(VOID) CONST
{
	if (m_File == INVALID_FILE)
		return FALSE;

	return TRUE;
}

BOOL DFile::Open(STRCPTR name, INT mode /* = OM_READ */)
{
	DAssert(m_File == INVALID_FILE);

	if (!name)
		return FALSE;

#ifdef WIN32
	DWORD access = 0UL;
	DWORD share = FILE_SHARE_READ;
	if (mode & OM_READ)
		access |= GENERIC_READ;
	if (mode & OM_WRITE) {
		share = 0UL;
		access |= GENERIC_WRITE;
	}
	if (!access)
		return FALSE;

	DWORD create = 0UL;
	if (mode & OM_CREATE) {
		if (mode & OM_WRITE)
			create = (mode & OM_TRUNCATE) ? CREATE_ALWAYS : OPEN_ALWAYS;
		else
			return FALSE;
	} else {
		if (mode & OM_TRUNCATE) {
			if (mode & OM_WRITE)
				create = TRUNCATE_EXISTING;
			else
				return FALSE;
		} else {
			create = OPEN_EXISTING;
		}
	}

	m_File = ::CreateFile(name, access, share, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_File == INVALID_HANDLE_VALUE)
		return FALSE;
#else
	STRCPTR fmt = NULL;

	if (!(mode & OM_WRITE)) {
		if (!(mode & OM_READ))
			return FALSE;
		if ((mode & OM_CREATE) || (mode & OM_TRUNCATE))
			return FALSE;
		fmt = "rb";
	} else {
		if (mode & OM_CREATE) {
			if (mode & OM_TRUNCATE) {
				fmt = (mode & OM_READ) ? "wb+" : "wb";
			} else {
				fmt = "ab";
			}
		} else {
			fmt = (mode & OM_TRUNCATE) ? "wb+" : "rb+";
		}
	}

	m_File = fopen(name, fmt);
	if (!m_File)
		return FALSE;
#endif

	m_Name = name;
	m_Temp = FALSE;
	return TRUE;
}

BOOL DFile::Close(VOID)
{
	if (m_File == INVALID_FILE)
		return FALSE;

#ifdef WIN32
	::CloseHandle(m_File);
	if (m_Temp)
		::DeleteFile(m_Name);
#else
	::fclose(m_File);
	if (m_Temp)
		::remove(m_Name);
#endif

	m_File = INVALID_FILE;
	m_Temp = FALSE;
	return TRUE;
}

BOOL DFile::CreateTempFile(VOID)
{
	DAssert(m_File == INVALID_FILE);

	// TODO:
	m_Temp = TRUE;
	return TRUE;
}

UINT DFile::Read(VPTR buf, UINT size)
{
	if (m_File == INVALID_FILE || !buf || !size)
		return 0U;

#ifdef WIN32
	DWORD rd_size = 0UL;
	::ReadFile(m_File, buf, size, &rd_size, NULL);
	return rd_size;
#else
	return fread(buf, 1, size, m_File);
#endif
}

UINT DFile::ReadLine(STRPTR buf, UINT size /* = 0U */)
{
	// TODO:
	return 0U;
}

UINT DFile::ReadFormat(STRPTR buf, STRCPTR fmt, ...)
{
	// TODO:
	return 0U;
}

UINT DFile::Write(VCPTR buf, UINT size)
{
	if (m_File == INVALID_FILE || !buf || !size)
		return 0U;

#ifdef WIN32
	DWORD wr_size = 0UL;
	::WriteFile(m_File, buf, size, &wr_size, NULL);
	return wr_size;
#else
	return fwrite(buf, 1, size, m_File);
#endif
}

UINT DFile::WriteLine(STRCPTR buf, UINT size /* = 0U */)
{
	// TODO:
	return 0U;
}

UINT DFile::WriteFormat(STRCPTR fmt, ...)
{
	// TODO:
	return 0U;
}

VOID DFile::Flush(VOID)
{
#ifdef WIN32
	::FlushFileBuffers(m_File);
#else
	::fflush(m_File);
#endif
}

UINT DFile::Position(VOID) CONST
{
#ifdef WIN32
	return ::SetFilePointer(m_File, 0L, NULL, FILE_CURRENT);
#else
	return ::ftell(m_File);
#endif
}

UINT DFile::Seek(INT offset, SEEK_MODE mode /* = SM_BEGIN */)
{
	if (m_File == INVALID_FILE)
		return ERROR_POS;

#ifdef WIN32
	return ::SetFilePointer(m_File, offset, NULL, mode);
#else
	if (!::fseek(m_File, offset, mode))
		return ::ftell(m_File);

	return ERROR_POS;
#endif
}

VOID DFile::Rewind(VOID)
{
	if (m_File == INVALID_FILE)
		return;

#ifdef WIN32
	::SetFilePointer(m_File, 0L, NULL, FILE_BEGIN);
#else
	::rewind(m_File);
#endif
}

UINT DFile::GetSize(VOID) CONST
{
	if (m_File == INVALID_FILE)
		return ERROR_SIZE;

#ifdef WIN32
	DWORD ret = ::GetFileSize(m_File, NULL);
	if (ret == INVALID_FILE_SIZE)
		return ERROR_SIZE;

	return ret;
#else
	LONG pos = ::ftell(m_File);
	if (pos < 0)
		return ERROR_SIZE;

	if (::fseek(m_File, 0, SEEK_END))
		return ERROR_SIZE;

	UINT ret = ::ftell(m_File);
	DVerify(!::fseek(m_File, pos, SEEK_SET));
	return ret;
#endif
}

BOOL DFile::SetSize(UINT size)
{
	if (m_File == INVALID_FILE)
		return FALSE;

	if (size == ERROR_SIZE)
		return FALSE;

#ifdef WIN32
	if (::SetFilePointer(m_File, size, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return FALSE;
	return ::SetEndOfFile(m_File);
#else
	return ::ftruncate(::fileno(m_File), size) != 0;
#endif
}

BOOL DFile::IsEnd(VOID) CONST
{
	if (m_File == INVALID_FILE)
		return TRUE;

#ifdef WIN32
	DWORD size = ::GetFileSize(m_File, NULL);
	if (size == INVALID_FILE_SIZE)
		return TRUE;

	DWORD pos = ::SetFilePointer(m_File, 0L, 0L, FILE_CURRENT);
	if (pos == INVALID_SET_FILE_POINTER)
		return TRUE;

	if (pos >= size)
		return TRUE;

	return FALSE;
#else
	return ::feof(m_File) != 0;
#endif
}

BOOL DFile::Attach(FILEHANDLE handle, STRCPTR name /* = NULL */)
{
	if (handle == INVALID_FILE)
		return FALSE;

	if (m_File != INVALID_FILE)
		return FALSE;

	if (name)
		m_Name = name;
	else
		m_Name.Clear();

	m_File = handle;
	return TRUE;
}

DFile::FILEHANDLE DFile::Detach(VOID)
{
	if (m_File == INVALID_FILE)
		return INVALID_FILE;

	FILEHANDLE file = m_File;
	m_Name.Clear();
	m_Temp = FALSE;
	m_File = INVALID_FILE;

	return file;
}

BOOL DFile::IsValidHandle(FILEHANDLE handle)
{
	if (handle == INVALID_FILE)
		return FALSE;

	return TRUE;
}

UINT DFile::GetFullPath(STRCPTR name, STRPTR buf /* = NULL */, UINT buf_size /* = 0U */)
{
	if (!name || !*name)
		return 0;

#ifdef WIN32
	return ::GetFullPathName(name, buf_size, buf, NULL);
#else
	// TODO:
#endif
}

BOOL DFile::IsExist(STRCPTR name)
{
	if (!name || !*name)
		return FALSE;

#ifdef WIN32
	if (::GetFileAttributes(name) == 0xffffffffUL)
		return FALSE;
#else
	if (::access(name, 0))
		return FALSE;
#endif

	return TRUE;
}

BOOL DFile::IsFile(STRCPTR name)
{
	if (!name || !*name)
		return FALSE;

#ifdef WIN32
	DWORD attr = ::GetFileAttributes(name);
	if (attr == 0xffffffffUL)
		return FALSE;
	if (attr & FILE_ATTRIBUTE_DEVICE || attr & FILE_ATTRIBUTE_DIRECTORY || attr & FILE_ATTRIBUTE_OFFLINE)
		return FALSE;
#else
	// TODO:
	return FALSE;
#endif

	return TRUE;
}

BOOL DFile::IsDir(STRCPTR name)
{
	if (!name || !*name)
		return FALSE;

#ifdef WIN32
	DWORD attr = ::GetFileAttributes(name);
	if (attr == 0xffffffffUL)
		return FALSE;
	if (!(attr & FILE_ATTRIBUTE_DIRECTORY) || attr & FILE_ATTRIBUTE_OFFLINE)
		return FALSE;
#else
	// TODO:
	return FALSE;
#endif

	return TRUE;
}

BOOL DFile::Remove(STRCPTR name)
{
	if (!name || !*name)
		return FALSE;

#ifdef WIN32
	if (!::DeleteFile(name))
		return FALSE;
#else
	if (::remove(name))
		return FALSE;
#endif

	return TRUE;
}

/************************************************************************/

DFile::DFile(CONST DFile &file)
{
	DAssert(FALSE);
}

DFile &DFile::operator = (CONST DFile &file)
{
	DAssert(FALSE);
	return *this;
}

/************************************************************************/
