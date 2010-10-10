/************************************************************************/
/* File Name   : image.cpp                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 22nd, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DImage class implementation                            */
/************************************************************************/

#include <image.hpp>

/************************************************************************/

DImage::DImage() :
	m_Attach(FALSE),
	m_DataSize(0U)
{
	DVarClr(m_Image);
}

DImage::DImage(IMAGE &img) :
	m_Attach(FALSE),
	m_DataSize(0U)
{
	DVarClr(m_Image);
	Attach(img);
}

DImage::~DImage()
{
	if (!m_Attach)
		FreeData(m_Image.data);
}

/************************************************************************/

BOOL DImage::IsValid(VOID) CONST
{
	return m_Image.data != NULL;
}

VOID DImage::GetSize(SIZE &size) CONST
{
	size = m_Image.size;
}

INT DImage::GetWidth(VOID) CONST
{
	return m_Image.size.cx;
}

INT DImage::GetHeight(VOID) CONST
{
	return m_Image.size.cy;
}

BOOL DImage::SetSize(CONST SIZE &size)
{
	if (!m_Image.data)
		return FALSE;

	if (size.cx <= 0 || size.cy <= 0)
		return FALSE;

	if (size.cx > static_cast<INT>(m_Image.pitch) || size.cy > static_cast<INT>(m_DataSize / m_Image.pitch))
		return FALSE;

	m_Image.size = size;
	return TRUE;
}

UINT DImage::GetPitch(VOID) CONST
{
	return m_Image.pitch;
}

BUFPTR DImage::GetData(UINT *size /* = NULL */)
{
	if (!m_Image.data)
		return NULL;

	if (size)
		*size = m_DataSize;

	return m_Image.data;
}

BUFCPTR DImage::GetData(UINT *size /* = NULL */) CONST
{
	if (!m_Image.data)
		return NULL;

	if (size)
		*size = m_DataSize;

	return m_Image.data;
}

IMGPTR DImage::GetImage(VOID)
{
	if (!m_Image.data)
		return NULL;

	return &m_Image;
}

IMGCPTR DImage::GetImage(VOID) CONST
{
	if (!m_Image.data)
		return NULL;

	return &m_Image;
}

BOOL DImage::Attach(IMAGE &img)
{
	return Attach(img.size, img.pitch, img.data, img.pitch * img.size.cy);
}

BOOL DImage::Attach(CONST SIZE &size, UINT pitch, BUFPTR data, UINT data_size)
{
	if (m_Image.data)
		return FALSE;

	if (!CheckImage(size, pitch, data, data_size))
		return FALSE;

	m_Attach = TRUE;
	m_Image.size = size;
	m_Image.pitch = pitch;
	m_Image.data = data;
	m_DataSize = data_size;
	return TRUE;
}

BOOL DImage::Detach(VOID)
{
	if (!m_Attach)
		return FALSE;

	m_Attach = FALSE;
	m_DataSize = 0U;
	DVarClr(m_Image);
	return TRUE;
}

BOOL DImage::Create(CONST SIZE &size, UINT pitch, BUFCPTR data, UINT data_size, BOOL shrink /* = FALSE */)
{
	if (m_Image.data)
		return FALSE;

	if (!pitch)
		pitch = size.cx;

	if (!CheckImage(size, pitch, data, data_size))
		return FALSE;

	if (shrink && pitch != size.cx) {
		UINT buf_size = size.cx * size.cy;
		BUFPTR buf = AllocData(buf_size);
		if (!buf)
			return FALSE;
		m_Image.data = buf;
		m_DataSize = buf_size;
		for (INT i = 0; i < size.cy; i++) {
			DMemCpy(buf, data, size.cx);
			buf += size.cx;
			data += pitch;
		}
	} else {
		UINT buf_size = pitch * size.cy;
		BUFPTR buf = AllocData(buf_size);
		if (!buf)
			return FALSE;
		DMemCpy(buf, data, buf_size);
		m_Image.data = buf;
		m_DataSize = buf_size;
	}

	m_Attach = FALSE;
	m_Image.size = size;
	m_Image.pitch = pitch;
	return TRUE;
}

BOOL DImage::Create(CONST SIZE &size, UINT pitch /* = 0U */, BOOL fill /* = FALSE */, BYTE fill_color /* = '\0' */)
{
	if (m_Image.data)
		return FALSE;

	if (size.cx <= 0 || size.cy <= 0)
		return FALSE;

	if (!pitch)
		pitch = size.cx;

	UINT buf_size = pitch * size.cy;
	BUFPTR buf = AllocData(buf_size);
	if (!buf)
		return FALSE;

	if (fill)
		DMemSet(buf, fill_color, buf_size);

	m_Attach = FALSE;
	m_Image.data = buf;
	m_DataSize = buf_size;
	m_Image.size = size;
	m_Image.pitch = pitch;
	return TRUE;
}

BOOL DImage::Destroy(VOID)
{
	if (m_Attach)
		return FALSE;

	FreeData(m_Image.data);
	m_DataSize = 0U;
	DVarClr(m_Image);
	return TRUE;
}

BOOL DImage::Copy(CONST DImage &img, UINT pitch /* = 0U */, BOOL shrink /* = FALSE */)
{
	return Copy(img.m_Image, pitch, shrink);
}

BOOL DImage::Copy(CONST IMAGE &img, UINT pitch /* = 0U */, BOOL shrink /* = FALSE */)
{
	if (pitch) {
		if (pitch < img.pitch)
			return FALSE;
		shrink = FALSE;
	} else {
		pitch = img.pitch;
	}

	return Create(img.size, pitch, img.data, img.pitch * img.size.cy, shrink);
}

BOOL DImage::Blit(CONST DImage &img, CONST POINT &pos)
{
	IMGCPTR p = img.GetImage();
	if (!p)
		return FALSE;

	return Blit(*p, pos);
}

BOOL DImage::Blit(CONST IMAGE &img, CONST POINT &pos)
{
	if (!IsValid() || !img.data)
		return FALSE;

	if (pos.x >= m_Image.size.cx || pos.y >= m_Image.size.cy)
		return TRUE;

	INT row;
	if (pos.x >= 0)
		row = DMin(img.size.cx, m_Image.size.cx - pos.x);
	else
		row = DMin(m_Image.size.cx, img.size.cx + pos.x);

	INT line;
	if (pos.y >= 0)
		line = DMin(img.size.cy, m_Image.size.cy - pos.y);
	else
		line = DMin(m_Image.size.cy, img.size.cy + pos.y);

	if (row < 0 || line < 0)
		return FALSE;

	if (!row || !line)
		return TRUE;

	BUFCPTR src = img.data + (pos.x < 0 ? -pos.x : 0) + (pos.y < 0 ? -pos.y : 0) * img.pitch;
	BUFPTR dest = m_Image.data + (pos.x > 0 ? pos.x : 0) + (pos.y > 0 ? pos.y : 0) * m_Image.pitch;
	for (INT i = 0; i < line; i++, src += img.pitch, dest += m_Image.pitch)
		DMemCpy(dest, src, row);

	return TRUE;
}

BOOL DImage::Blit(CONST DImage &img, CONST POINT &pos, BYTE color_key)
{
	IMGCPTR p = img.GetImage();
	if (!p)
		return FALSE;

	return Blit(*p, pos, color_key);
}

BOOL DImage::Blit(CONST IMAGE &img, CONST POINT &pos, BYTE color_key)
{
	if (!IsValid() || !img.data)
		return FALSE;

	if (pos.x >= m_Image.size.cx || pos.y >= m_Image.size.cy)
		return TRUE;

	INT row;
	if (pos.x >= 0)
		row = DMin(img.size.cx, m_Image.size.cx - pos.x);
	else
		row = DMin(m_Image.size.cx, img.size.cx + pos.x);

	INT line;
	if (pos.y >= 0)
		line = DMin(img.size.cy, m_Image.size.cy - pos.y);
	else
		line = DMin(m_Image.size.cy, img.size.cy + pos.y);

	if (row < 0 || line < 0)
		return FALSE;

	if (!row || !line)
		return TRUE;

	BUFCPTR src = img.data + (pos.x < 0 ? -pos.x : 0) + (pos.y < 0 ? -pos.y : 0) * img.pitch;
	BUFPTR dest = m_Image.data + (pos.x > 0 ? pos.x : 0) + (pos.y > 0 ? pos.y : 0) * m_Image.pitch;
	for (INT i = 0; i < line; i++, src += img.pitch, dest += m_Image.pitch) {
		for (INT j = 0; j < row; j++) {
			if (src[j])
				dest[j] = src[j];
		}
	}

	return TRUE;
}

BOOL DImage::Clear(BYTE fill_color /* = '\0' */)
{
	if (!m_Image.data)
		return FALSE;

	DMemSet(m_Image.data, fill_color, m_DataSize);
	return TRUE;
}

BOOL DImage::Offset(CONST POINT &pos, DImage &img) CONST
{
	img.Destroy();

	if (!m_Image.data)
		return FALSE;

	if (pos.x >= m_Image.size.cx || pos.y >= m_Image.size.cy)
		return FALSE;

	IMAGE image;
	image.pitch = m_Image.pitch;
	image.data = m_Image.data + pos.x + pos.y * m_Image.pitch;
	image.size.cx = m_Image.size.cx - pos.x;
	image.size.cy = m_Image.size.cy - pos.y;

	return img.Attach(image);
}

DImage::operator IMGPTR ()
{
	return &m_Image;
}

DImage::operator IMGCPTR () CONST
{
	return &m_Image;
}

/************************************************************************/

BUFPTR DImage::AllocData(UINT size)
{
	if (!size)
		return NULL;

	return new BYTE[size];
}

VOID DImage::FreeData(BUFPTR data)
{
	delete [] data;
}

BOOL DImage::CheckImage(SIZE size, UINT pitch, BUFCPTR data, UINT data_size)
{
	if (size.cx <= 0 || size.cy <= 0)
		return FALSE;

	if (pitch < static_cast<UINT>(size.cx))
		return FALSE;

	if (!data || data_size < pitch * size.cy)
		return FALSE;

	return TRUE;
}

/************************************************************************/

DImage::DImage(CONST DImage &img)
{
	DAssert(FALSE);
}

DImage &DImage::operator = (CONST DImage &img)
{
	DAssert(FALSE);
	return *this;
}

/************************************************************************/
