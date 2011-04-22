/************************************************************************/
/* File Name   : pcx.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 15th, 2008                                         */
/* Module      : Lawine library                                         */
/* Descript    : DPcx class implementation                              */
/************************************************************************/

#include <array.hpp>
#include "pcx.hpp"
#include "../global.hpp"

/************************************************************************/

CONST BYTE SUPPORT_MANUFACTURER = 0x0a;
CONST BYTE SUPPORT_VERSION = 5;
CONST BYTE SUPPORT_ENCODING = 1;
CONST BYTE SUPPORT_DEPTH = D_COLOR_DEPTH;
CONST BYTE SUPPORT_PLANES = 1;
CONST WORD SUPPORT_PALINFO = 1;

CONST BYTE PALETTE_FLAG = 0x0c;
CONST BYTE RLE_MARK = 0xc0;

/************************************************************************/

DPcx::DPcx() :
	m_DataBuf(NULL),
	m_DataSize(0U)
{

}

DPcx::~DPcx()
{
	Clear();
}

/************************************************************************/

BOOL DPcx::Create(CONST DImage &img, CONST DPalette &pal)
{
	if (!img.IsValid() || !pal.IsValid())
		return FALSE;

	if (m_Image.IsValid())
		return FALSE;

	if (!m_Palette.IsValid())
		return FALSE;

	UINT pitch = img.GetWidth();
	if (pitch % 2)
		pitch++;

	if (!m_Image.Copy(img, pitch))
		return FALSE;

	m_Palette = pal;
	return TRUE;
}

BOOL DPcx::Load(STRCPTR name)
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
	if (::g_Archive.ReadFile(file, data, size) == size) {
		m_DataBuf = data;
		m_DataSize = size;
		ret = TRUE;
	}

	::g_Archive.CloseFile(file);
	return ret;
}

BOOL DPcx::Save(STRCPTR name) CONST
{
	if (!name)
		return FALSE;

	DFile file;
	if (!file.Open(name, DFile::OM_WRITE | DFile::OM_CREATE | DFile::OM_TRUNCATE))
		return FALSE;

	if (m_DataBuf && m_DataSize) {
		if (file.Write(m_DataBuf, m_DataSize) != m_DataSize)
			return FALSE;
		return TRUE;
	}

	if (!m_Image.IsValid())
		return FALSE;

	UINT buf_size = m_Image.GetPitch() * m_Image.GetHeight() * 2;
	DArray<BYTE> buf(buf_size);
	if (buf.IsNull())
		return FALSE;

	return Encode(file, buf, buf_size);
}

VOID DPcx::Clear(VOID)
{
	m_Image.Destroy();

	delete [] m_DataBuf;
	m_DataBuf = NULL;
	m_DataSize = 0U;
}

BOOL DPcx::Decode(VOID)
{
	if (m_Image.IsValid())
		return TRUE;

	if (!m_DataBuf || !m_DataSize)
		return FALSE;

	CONST HEADER *head;
	CONST PALETTE *pal;
	BUFCPTR img;
	if (!CheckHeader(head, img, pal))
		return FALSE;

	SIZE size;
	size.cx = head->xmax - head->xmin + 1;
	size.cy = head->ymax - head->ymin + 1;
	if (!m_Image.Create(size, head->pitch, FALSE))
		return FALSE;

	BUFCPTR rle = m_DataBuf + sizeof(HEADER);
	UINT rle_size = m_DataSize - sizeof(HEADER) - sizeof(PALETTE);

	UINT data_size;
	BUFPTR data = m_Image.GetData(&data_size);
	DAssert(data && data_size);

	if (!DecodeRLE(rle, rle_size, data, data_size)) {
		m_Image.Destroy();
		return FALSE;
	}

	CLRPTR color = m_Palette;
	for (INT i = 0; i < D_COLOR_NUM; i++, color++) {
		color->red = pal->colors[i].red;
		color->green = pal->colors[i].green;
		color->blue = pal->colors[i].blue;
	}

	delete [] m_DataBuf;
	m_DataBuf = NULL;
	m_DataSize = 0U;
	return TRUE;
}

CONST DImage *DPcx::GetImage(VOID) CONST
{
	if (!m_Image.IsValid())
		return NULL;

	return &m_Image;
}

CONST DPalette *DPcx::GetPalette(VOID) CONST
{
	if (!m_Image.IsValid())
		return NULL;

	if (!m_Palette.IsValid())
		return NULL;

	return &m_Palette;
}

/************************************************************************/

BOOL DPcx::CheckHeader(CONST HEADER *&head, BUFCPTR &img, CONST PALETTE *&pal) CONST
{
	DAssert(m_DataBuf);

	if (m_DataSize <= sizeof(HEADER) + sizeof(PALETTE))
		return FALSE;

	head = reinterpret_cast<CONST HEADER *>(m_DataBuf);
	if (head->manufacturer != SUPPORT_MANUFACTURER)
		return FALSE;
	if (head->version != SUPPORT_VERSION)
		return FALSE;
	if (head->encoding != SUPPORT_ENCODING)
		return FALSE;
	if (head->depth != SUPPORT_DEPTH)
		return FALSE;
	if (head->planes != SUPPORT_PLANES)
		return FALSE;
	if (!head->pitch)
		return FALSE;

	img = m_DataBuf + sizeof(HEADER);
	pal = reinterpret_cast<CONST PALETTE *>(m_DataBuf + m_DataSize - sizeof(PALETTE));
	if (pal->flag != PALETTE_FLAG)
		return FALSE;

	return TRUE;
}

BOOL DPcx::Encode(DFile &file, BUFPTR buf, UINT buf_size) CONST
{
	DAssert(buf && buf_size);

	BUFCPTR data = m_Image.GetData();
	INT line = m_Image.GetHeight();
	UINT pitch = m_Image.GetPitch();
	DAssert(data && line && pitch);

	BUFPTR p = buf;
	UINT size = buf_size;
	for (; line > 0; line--, data += pitch) {
		UINT rle_size = EncodeRLE(data, pitch, p, buf_size);
		if (!rle_size)
			return FALSE;
		p += rle_size;
		buf_size -= rle_size;
	}

	size -= buf_size;

	HEADER header;
	DVarClr(header);
	header.manufacturer = SUPPORT_MANUFACTURER;
	header.version = SUPPORT_VERSION;
	header.encoding = SUPPORT_ENCODING;
	header.depth = SUPPORT_DEPTH;
	header.xmax = m_Image.GetWidth() - 1;
	header.ymax = m_Image.GetHeight() - 1;
	header.hor_dpi = m_Image.GetWidth();
	header.ver_dpi = m_Image.GetHeight();
	header.planes = SUPPORT_PLANES;
	header.pitch = m_Image.GetPitch();
	header.pal_info = SUPPORT_PALINFO;

	PALETTE pal;
	pal.flag = PALETTE_FLAG;
	CLRCPTR color = m_Palette;
	for (INT i = 0; i < D_COLOR_NUM; i++, color++) {
		pal.colors[i].red = color->red;
		pal.colors[i].green = color->green;
		pal.colors[i].blue = color->blue;
	}

	if (file.Write(&header, sizeof(header)) != sizeof(header))
		return FALSE;

	if (file.Write(buf, size) != size)
		return FALSE;

	if (file.Write(&pal, sizeof(pal)) != sizeof(pal))
		return FALSE;

	return TRUE;
}

UINT DPcx::EncodeRLE(BUFCPTR src, UINT src_size, BUFPTR dest, UINT dest_size)
{
	DAssert(src && src_size && dest && dest_size);

	UINT size = dest_size;

	do {

		BYTE color = *src++;
		UINT len = 1U;

		if (!--src_size || *src != color) {
			if (color < RLE_MARK) {
				*dest++ = color;
				dest_size--;
				continue;
			}
		} else {
			do {
				len++;
				src++;
				src_size--;
			} while (src_size && color == *src && len < (BYTE)~RLE_MARK);
		}

		if (dest_size < 2)
			return 0U;

		*dest++ = RLE_MARK | len;
		*dest++ = color;
		dest_size -= 2;

	} while (src_size && dest_size);

	if (src_size)
		return 0U;

	return size - dest_size;
}

UINT DPcx::DecodeRLE(BUFCPTR src, UINT src_size, BUFPTR dest, UINT dest_size)
{
	DAssert(src && src_size && dest && dest_size);

	UINT size = dest_size;

	do {

		BYTE color = *src++;
		src_size--;

		if (color < RLE_MARK) {
			*dest++ = color;
			if (!--dest_size)
				break;
			continue;
		}

		if (!src_size)
			break;

		UINT len = color & ~RLE_MARK;
		if (dest_size < len)
			return 0U;

		color = *src++;
		src_size--;

		dest_size -= len;
		for (UINT i = 0U; i < len; i++, dest++)
			*dest = color;

	} while (src_size && dest_size);

	return size - dest_size;
}

/************************************************************************/
