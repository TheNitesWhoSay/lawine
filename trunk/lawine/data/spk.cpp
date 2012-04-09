/************************************************************************/
/* File Name   : spk.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 15th, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : DSpk class implementation                              */
/************************************************************************/

#include <lawinedef.h>
#include <array.hpp>
#include "spk.hpp"
#include "../global.hpp"

/************************************************************************/

DSpk::DSpk() :
	m_LayerNum(0),
	m_Layer(NULL)
{

}

DSpk::~DSpk()
{
	Clear();
}

/************************************************************************/

BOOL DSpk::Load(STRCPTR name)
{
	if (m_Layer)
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

	DArray<BYTE> data(size);
	BOOL ret = FALSE;
	if (::g_Archive.ReadFile(file, data, size) == size) {
		if (Load(data, size))
			ret = TRUE;
		else
			Clear();
	}

	::g_Archive.CloseFile(file);
	return ret;
}

VOID DSpk::Clear(VOID)
{
	if (!m_Layer) {
		m_LayerNum = 0;
		return;
	}

	for (INT i = 0; i < m_LayerNum; i++)
		m_Layer[i].Destroy();

	delete [] m_Layer;
	m_Layer = NULL;
	m_LayerNum = 0;
}

INT DSpk::GetLayerNum(VOID) CONST
{
	return m_LayerNum;
}

BOOL DSpk::GetImage(DImage &img, CONST POINT &view_pos) CONST
{
	if (!img.IsValid())
		return FALSE;

	if (img.GetWidth() < L_SPK_WIDTH || img.GetHeight() < L_SPK_HEIGHT)
		return FALSE;

	img.Clear();

	for (INT i = 0; i < m_LayerNum; i++) {

		POINT pos;
		INT x = view_pos.x / (i + 2) % L_SPK_WIDTH;
		INT y = view_pos.y / (i + 2) % L_SPK_HEIGHT;

		pos.x = -x;
		pos.y = -y;
		img.Blit(m_Layer[i], pos, 0);

		pos.x = -x;
		pos.y = L_SPK_HEIGHT - y;
		img.Blit(m_Layer[i], pos, 0);

		pos.x = L_SPK_WIDTH - x;
		pos.y = -y;
		img.Blit(m_Layer[i], pos, 0);

		pos.x = L_SPK_WIDTH - x;
		pos.y = L_SPK_HEIGHT - y;
		img.Blit(m_Layer[i], pos, 0);
	}

	return TRUE;
}

CONST DImage *DSpk::GetImage(INT layer) CONST
{
	if (!DBetween(layer, 0, m_LayerNum))
		return NULL;

	DAssert(m_Layer);
	if (!m_Layer[layer].IsValid())
		return NULL;

	return &m_Layer[layer];
}

/************************************************************************/

BOOL DSpk::Load(BUFCPTR data, UINT size)
{
	DAssert(data && size);
	DAssert(!m_Layer);

	WORD layer_num;
	CONST WORD *frame_num;
	CONST FRAMEHEADER *frame_head;
	if (!CheckHeader(data, size, layer_num, frame_num, frame_head))
		return FALSE;

	m_LayerNum = layer_num;
	m_Layer = new DImage[m_LayerNum];

	for (INT i = 0; i < m_LayerNum; i++, frame_num++) {
		LoadLayer(i, *frame_num, frame_head, data, size);
		frame_head += *frame_num;
	}

	return TRUE;
}

BOOL DSpk::LoadLayer(INT layer, WORD frame_num, CONST FRAMEHEADER *frame_head, BUFCPTR data, UINT size)
{
	DAssert(DBetween(layer, 0, m_LayerNum));
	DAssert(frame_num && frame_head && data && size && m_Layer);

	BOOL ret;
	DImage *image = &m_Layer[layer];
	SIZE spk_size = { L_SPK_WIDTH, L_SPK_HEIGHT };
	if (!image->Create(spk_size))
		return FALSE;

	for (INT i = 0; i < frame_num; i++, frame_head++) {

		ret = FALSE;

		UINT offset = frame_head->bmp_offset;
		if (!offset || offset > size)
			break;

		CONST FRAMEINFO *info = reinterpret_cast<CONST FRAMEINFO *>(data + offset);
		if (!info->width || !info->height)
			break;
		offset += sizeof(FRAMEINFO);

		UINT data_size = info->width * info->height;
		if (offset + data_size > size)
			break;

		IMAGE img;
		img.size.cx = info->width;
		img.size.cy = info->height;
		img.data = const_cast<BUFPTR>(data + offset);
		img.pitch = info->width;

		POINT pos;
		pos.x = frame_head->hor_offset % L_SPK_WIDTH;
		pos.y = frame_head->ver_offset % L_SPK_HEIGHT;
		if (!image->Blit(&img, pos))
			break;

		ret = TRUE;
	}

	if (!ret) {
		Clear();
		return FALSE;
	}

	return TRUE;
}

BOOL DSpk::CheckHeader(BUFCPTR data, UINT size, WORD &layer_num, CONST WORD *&frame_num, CONST FRAMEHEADER *&frame_head)
{
	DAssert(data && size);

	UINT offset = 0U;

	DMemCpy(&layer_num, data, sizeof(layer_num));
	offset += sizeof(WORD);
	if (!layer_num || offset > size)
		return FALSE;

	frame_num = reinterpret_cast<CONST WORD *>(data + offset);
	offset += layer_num * sizeof(WORD);
	if (offset > size)
		return FALSE;

	UINT all_bmp_num = 0U;
	for (UINT i = 0; i < layer_num; i++)
		all_bmp_num += frame_num[i];
	if (!all_bmp_num)
		return FALSE;

	frame_head = reinterpret_cast<CONST FRAMEHEADER *>(data + offset);
	offset += all_bmp_num * sizeof(FRAMEHEADER);
	if (offset > size)
		return FALSE;

	return TRUE;
}

/************************************************************************/
