/************************************************************************/
/* File Name   : smk.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 10th, 2009                                         */
/* Module      : Lawine library                                         */
/* Descript    : DSmk class implementation                              */
/************************************************************************/

#include "smk.hpp"
#include "../global.hpp"

/************************************************************************/

CONST INT SUPPORT_PALETTETYPE = 0;

/************************************************************************/

DSmk::DSmk() :
	m_Loop(0),
	m_Play(FALSE),
	m_Pause(FALSE),
	m_Smack(NULL)
{

}

DSmk::~DSmk()
{
	Clear();
}

BOOL DSmk::Load(STRCPTR name)
{
	if (m_Smack || m_File.IsOpen())
		return FALSE;

	if (!name)
		return FALSE;

	HANDLE handle = ::g_Archive.OpenHandle(name);
	if (!handle)
		return FALSE;

	DFile file;
	DVerify(file.Attach(handle));

	/* 用WIN32句柄打开SMK视频 */
	SMACK *smk = ::SmackOpen(reinterpret_cast<STRCPTR>(handle), SMACK_FILEHANDLE | SMACK_TRACKS, SMACK_AUTOEXTRA);
	if (!smk)
		return FALSE;

	SIZE size;
	size.cx = smk->width;
	size.cy = smk->height;
	if (!m_Image.Create(size)) {
		::SmackClose(smk);
		return FALSE;
	}

	m_Smack = smk;
	m_File.Attach(file.Detach());
	return TRUE;
}

VOID DSmk::Clear(VOID)
{
	m_Image.Destroy();

	::SmackClose(m_Smack);
	m_Smack = NULL;

	m_File.Close();

	m_Loop = 0;
	m_Play = FALSE;
	m_Pause = FALSE;
}

BOOL DSmk::Play(VOID)
{
	if (!m_Smack)
		return FALSE;

	if (!m_Play || m_Pause)
		return TRUE;

	/* 如果SMK换帧时机未到，什么都不作 */
	if (::SmackWait(m_Smack))
		return TRUE;

	BUFPTR data = m_Image.GetData();
	if (!data)
		return TRUE;

	::SmackToBuffer(m_Smack, 0, 0, m_Image.GetPitch(), m_Image.GetHeight(), data, SMACK_BUFFMT_256);
	::SmackDoFrame(m_Smack);

	if (m_Smack->new_palette)
		TransferPalette(m_Smack->pal_type, m_Smack->palette);

	/* SMK换帧和循环播放处理 */
	if (m_Smack->frame_no != m_Smack->frame_num - 1) {
		::SmackNextFrame(m_Smack);
		return TRUE;
	}

	if (!m_Loop) {
		::SmackGoto(m_Smack, 0);
		return TRUE;
	}

	m_Loop--;

	if (!m_Loop)
		return Stop();

	::SmackGoto(m_Smack, 0);
	return TRUE;
}

BOOL DSmk::Start(INT loop /* = 0 */)
{
	if (!m_Smack)
		return FALSE;

	if (loop <= 0)
		m_Loop = 0;
	else
		m_Loop = loop;

	m_Play = TRUE;
	m_Pause = FALSE;
	::SmackGoto(m_Smack, 0);
	return TRUE;
}

BOOL DSmk::Stop(VOID)
{
	if (!m_Smack)
		return FALSE;

	m_Loop = 0;
	m_Pause = FALSE;
	m_Play = FALSE;
	::SmackGoto(m_Smack, 0);
	return TRUE;
}

BOOL DSmk::Pause(VOID)
{
	if (!m_Smack)
		return FALSE;

	m_Pause = TRUE;
	return TRUE;
}

BOOL DSmk::Resume(VOID)
{
	if (!m_Smack)
		return FALSE;

	m_Pause = FALSE;
	return TRUE;
}

INT DSmk::GetFrameNum(VOID) CONST
{
	if (!m_Smack)
		return 0;

	return m_Smack->frame_num;
}

INT DSmk::GetCurFrame(VOID) CONST
{
	if (!m_Smack)
		return -1;

	return m_Smack->frame_no;
}

INT DSmk::GetRestLoop(VOID) CONST
{
	return m_Loop;
}

BOOL DSmk::GetSize(SIZE &size) CONST
{
	if (!m_Smack) {
		size.cx = 0;
		size.cy = 0;
		return FALSE;
	}

	size.cx = m_Smack->width;
	size.cy = m_Smack->height;
	return TRUE;
}

CONST DImage *DSmk::GetImage(VOID) CONST
{
	if (!m_Smack)
		return NULL;

	return &m_Image;
}

/************************************************************************/

VOID DSmk::TransferPalette(INT type, BUFCPTR pal)
{
	DAssert(type == SUPPORT_PALETTETYPE);

	CLRPTR color = m_Palette;

	for (INT i = 0; i < D_COLOR_NUM; i++, color++) {
		color->red = *pal++;
		color->green = *pal++;
		color->blue = *pal++;
	}
}

/************************************************************************/
