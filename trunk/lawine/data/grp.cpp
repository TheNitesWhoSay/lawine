/************************************************************************/
/* File Name   : grp.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 6th, 2009                                          */
/* Module      : Lawine library                                         */
/* Descript    : DGrp class implementation                              */
/************************************************************************/

#include "grp.hpp"
#include "../global.hpp"

/************************************************************************/

DGrp::DGrp() :
	m_FrameList(NULL),
	m_FrameNum(0),
	m_DataBuf(NULL),
	m_DataSize(0U),
	m_CacheFrame(-1)
{

}

DGrp::~DGrp()
{
	Clear();
}

/************************************************************************/

BOOL DGrp::Load(STRCPTR name)
{
	if (m_Image.IsValid())
		return FALSE;

	if (!name)
		return FALSE;

	HANDLE file = ::g_Archive.OpenFile(name);
	if (!file)
		return FALSE;

	UINT size = ::g_Archive.GetFileSize(file);
	if (!size) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	BUFPTR data = new BYTE[size];
	BOOL ret = FALSE;
	if (data && ::g_Archive.ReadFile(file, data, size) == size)
		ret = Load(data, size);

	if (!ret) {
		delete [] data;
		Clear();
	}

	::g_Archive.CloseFile(file);
	return ret;
}

VOID DGrp::Clear(VOID)
{
	m_Image.Destroy();

	for (INT i = 0; i < m_FrameNum; i++)
		delete m_FrameList[i];

	delete [] m_FrameList;
	m_FrameList = NULL;
	m_FrameNum = 0;

	delete [] m_DataBuf;
	m_DataBuf = NULL;
	m_DataSize = 0U;
	
	m_CacheFrame = -1;
}

BOOL DGrp::Decode(INT frame_no)
{
	if (!DBetween(frame_no, 0, m_FrameNum))
		return FALSE;

	if (frame_no == m_CacheFrame)
		return TRUE;

	DFrame *frame = m_FrameList[frame_no];
	if (!frame)
		return FALSE;

	frame->Decode();
	m_CacheFrame = frame_no;
	return TRUE;
}

INT DGrp::GetFrameNum(VOID) CONST
{
	return m_FrameNum;
}

CONST DImage *DGrp::GetImage(VOID) CONST
{
	return &m_Image;
}

/************************************************************************/

BOOL DGrp::Load(BUFCPTR data, UINT size)
{
	CONST HEADER *head;
	CONST FRAMEHEADER *frame_head;
	if (!CheckHeader(data, size, head, frame_head))
		return FALSE;

	SIZE grp_size;
	grp_size.cx = head->width;
	grp_size.cy = head->height;
	if (!m_Image.Create(grp_size))
		return FALSE;

	m_DataBuf = data;
	m_DataSize = size;

	m_FrameNum = head->frame_num;
	m_FrameList = new (DFrame *[m_FrameNum]);
	if (!m_FrameList)
		return FALSE;
	DMemClr(m_FrameList, m_FrameNum * sizeof(DFrame *));

	UINT pitch = head->width;
	for (INT i = 0; i < m_FrameNum; i++)
		m_FrameList[i] = LoadFrame(frame_head + i, pitch);

	return TRUE;
}

DGrp::DFrame *DGrp::LoadFrame(CONST FRAMEHEADER *frame_head, UINT pitch)
{
	DAssert(frame_head && m_DataBuf);

	BUFPTR data = m_Image.GetData();
	if (!data)
		return NULL;

	DFrame *frame = new DFrame(data, pitch);
	if (!frame->Load(frame_head, m_DataBuf)) {
		delete frame;
		frame = NULL;
	}

	return frame;
}

BOOL DGrp::CheckHeader(BUFCPTR data, UINT size, CONST HEADER *&head, CONST FRAMEHEADER *&frame_head)
{
	if (!data)
		return FALSE;

	if (size <= sizeof(HEADER))
		return FALSE;

	head = reinterpret_cast<CONST HEADER *>(data);
	if (!head->frame_num || !head->width || !head->height)
		return FALSE;

	if (size <= head->frame_num * sizeof(FRAMEHEADER))
		return FALSE;

	frame_head = reinterpret_cast<CONST FRAMEHEADER *>(data + sizeof(HEADER));
	return TRUE;
}

/************************************************************************/

DGrp::DFrame::DFrame(BUFPTR buf, UINT pitch) :
	m_Buffer(buf),
	m_Pitch(pitch),
	m_Header(NULL),
	m_LineOffset(NULL)
{
	DAssert(buf && pitch);
}

DGrp::DFrame::~DFrame()
{
	Clear();
}

BOOL DGrp::DFrame::Load(CONST FRAMEHEADER *head, BUFCPTR data)
{
	if (m_LineOffset)
		return FALSE;

	if (!head || !data)
		return FALSE;

	if (!CheckHeader(head))
		return FALSE;

	m_Header = head;
	m_LineOffset = reinterpret_cast<CONST LINEOFFSET *>(data + head->line_offset);
	return TRUE;
}

VOID DGrp::DFrame::Clear(VOID)
{
	m_Header = NULL;
	m_LineOffset = NULL;
}

VOID DGrp::DFrame::Decode(VOID)
{
	DAssert(m_Header && m_LineOffset);

	BUFPTR buf = m_Buffer + m_Header->hor_offset + m_Header->ver_offset * m_Pitch;
	BUFCPTR data_off = reinterpret_cast<BUFCPTR>(m_LineOffset);

	for (INT i = 1; i < m_Header->line_num; i++, buf += m_Pitch) {
		BUFCPTR data_begin = data_off + m_LineOffset[i - 1];
		BUFCPTR data_end = data_off + m_LineOffset[i];
		DecodeLine(data_begin, data_end, buf);
	}
}

BOOL DGrp::DFrame::CheckHeader(CONST FRAMEHEADER *head)
{
	DAssert(head);

	if (!head->line_num || !head->pixel_per_line || !head->line_offset)
		return FALSE;

	return TRUE;
}

VOID DGrp::DFrame::DecodeLine(BUFCPTR line_begin, BUFCPTR line_end, BUFPTR buf)
{
	DAssert(line_begin && line_end && buf);

	for (BUFCPTR p = line_begin; p < line_end; p++) {
		if (*p >= 0x80) {
			buf += *p & 0x7f;
		} else if (*p <= 0x40) {
			for (INT i = *p; i > 0; i--)
				*buf++ = *++p;
		} else {
			for (INT i = *p++ & 0x3f; i > 0; i--)
				*buf++ = *p;
		}
	}
}

/************************************************************************/
