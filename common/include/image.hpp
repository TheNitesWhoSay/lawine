/************************************************************************/
/* File Name   : image.hpp                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 22nd, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DImage class declaration                               */
/************************************************************************/

#ifndef __SD_COMMON_IMAGE_HPP__
#define __SD_COMMON_IMAGE_HPP__

/************************************************************************/

#include <common.h>
#include <image.h>

/************************************************************************/

class DImage {

public:

	DImage();
	explicit DImage(IMAGE &img);
	~DImage();

	BOOL IsValid(VOID) CONST;
	INT GetWidth(VOID) CONST;
	INT GetHeight(VOID) CONST;
	VOID GetSize(SIZE &size) CONST;
	BOOL SetSize(CONST SIZE &size);
	UINT GetPitch(VOID) CONST;
	BUFPTR GetData(UINT *size = NULL);
	BUFCPTR GetData(UINT *size = NULL) CONST;
	IMGPTR GetImage(VOID);
	IMGCPTR GetImage(VOID) CONST;

	BOOL Attach(IMAGE &img);
	BOOL Attach(CONST SIZE &size, UINT pitch, BUFPTR data, UINT data_size);
	BOOL Detach(VOID);
	BOOL Create(CONST SIZE &size, UINT pitch, BUFCPTR data, UINT data_size, BOOL shrink = FALSE);
	BOOL Create(CONST SIZE &size, UINT pitch = 0U, BOOL fill = TRUE, BYTE fill_color = '\0');
	BOOL Destroy(VOID);
	BOOL Copy(IMGCPTR img, UINT pitch = 0U, BOOL shrink = FALSE);
	BOOL Blit(IMGCPTR img, CONST POINT &pos);
	BOOL Blit(IMGCPTR img, CONST POINT &pos, CONST SIZE &size);
	BOOL Blit(IMGCPTR img, CONST POINT &pos, BYTE color_key);
	BOOL Clear(BYTE fill_color = '\0');
	BOOL Offset(CONST POINT &pos, DImage &img) CONST;

	operator IMGPTR ();
	operator IMGCPTR () CONST;

protected:

	static BUFPTR AllocData(UINT size);
	static VOID FreeData(BUFPTR data);
	static BOOL CheckImage(SIZE size, UINT pitch, BUFCPTR data, UINT data_size);

	BOOL		m_Attach;
	UINT		m_DataSize;
	IMAGE		m_Image;

private:

	explicit DImage(CONST DImage &img);
	DImage &operator = (CONST DImage &img);

};

/************************************************************************/

#endif	/* __SD_COMMON_IMAGE_HPP__ */
