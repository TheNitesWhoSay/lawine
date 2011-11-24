/************************************************************************/
/* File Name   : archive.cpp                                            */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 23rd, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : DArchive class implementation                          */
/************************************************************************/

#include "archive.hpp"

/************************************************************************/

DArchive::DArchive()
{

}

DArchive::~DArchive()
{
	for (DArcList::iterator it = m_ArcList.begin(); it != m_ArcList.end(); ++it)
		delete it->mpq;
}

/************************************************************************/

DMpq *DArchive::UseArchive(STRCPTR mpq_name, UINT priority /* = 0U */)
{
	if (!mpq_name)
		return NULL;

	DMpq *mpq = new DMpq;
	if (!mpq->OpenArchive(mpq_name)) {
		delete mpq;
		return NULL;
	}

	ARCHIVE archive;
	archive.mpq = mpq;
	archive.priority = priority;

	// 根据优先级有序插入
	DArcList::iterator it = m_ArcList.begin();
	for (; it != m_ArcList.end(); ++it) {
		if (it->priority < priority)
			break;
	}

	m_ArcList.insert(it, archive);
	return mpq;
}

BOOL DArchive::CloseArchive(DMpq *mpq)
{
	if (!mpq)
		return FALSE;

	for (DArcList::iterator it = m_ArcList.begin(); it != m_ArcList.end(); ++it) {
		if (it->mpq == mpq) {
			m_ArcList.erase(it);
			break;
		}
	}

	BOOL ret = mpq->CloseArchive();
	delete mpq;
	return ret;
}

BOOL DArchive::FileExist(STRCPTR file_name)
{
	if (!SearchFile(file_name))
		return FALSE;

	return TRUE;
}

HANDLE DArchive::OpenFile(STRCPTR file_name)
{
	DMpq *mpq = SearchFile(file_name);
	if (!mpq)
		return NULL;

	HANDLE file = mpq->OpenFile(file_name);
	if (!file)
		return NULL;

	return file;
}

BOOL DArchive::CloseFile(HANDLE file)
{
	DMpq *mpq = SearchFile(file);
	if (!mpq)
		return FALSE;

	DVerify(mpq->CloseFile(file));
	return TRUE;
}

UINT DArchive::GetFileSize(HANDLE file)
{
	return DMpq::GetFileSize(file);
}

UINT DArchive::ReadFile(HANDLE file, VPTR data, UINT size)
{
	return DMpq::ReadFile(file, data, size);
}

UINT DArchive::SeekFile(HANDLE file, INT offset, SEEK_MODE mode /* = SM_BEGIN */)
{
	return DMpq::SeekFile(file, offset, mode);
}

HANDLE DArchive::OpenHandle(STRCPTR file_name)
{
	DMpq *mpq = SearchFile(file_name);
	if (!mpq)
		return NULL;

	HANDLE file = mpq->OpenHandle(file_name);
	if (!file)
		return NULL;

	return file;
}

/************************************************************************/

DMpq *DArchive::SearchFile(STRCPTR file_name) CONST
{
	if (!file_name)
		return NULL;

	for (DArcList::const_iterator it = m_ArcList.begin(); it != m_ArcList.end(); ++it) {
		if (it->mpq && it->mpq->FileExist(file_name))
			return it->mpq;
	}

	return NULL;
}

DMpq *DArchive::SearchFile(HANDLE file) CONST
{
	if (!file)
		return NULL;

	for (DArcList::const_iterator it = m_ArcList.begin(); it != m_ArcList.end(); ++it) {
		if (it->mpq && it->mpq->FileExist(file))
			return it->mpq;
	}

	return NULL;
}

/************************************************************************/
