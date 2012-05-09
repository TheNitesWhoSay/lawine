/************************************************************************/
/* File Name   : fnt.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 6th, 2009                                          */
/* Module      : Lawine library                                         */
/* Descript    : DFnt class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_FNT_HPP__
#define __SD_LAWINE_DATA_FNT_HPP__

/************************************************************************/

#include <common.h>
#include <image.hpp>
#include "pcx.hpp"

/************************************************************************/

class DFnt {

public:

	static CONST INT STYLE_NUM = 6;

	DFnt();
	~DFnt();

	BOOL GetMaxSize(SIZE &size) CONST;
	BOOL Load(STRCPTR name);
	VOID Clear(VOID);
	BOOL SetTemplate(IMGCPTR temp);
	BOOL GetCharSize(BYTE ch, SIZE &size) CONST;
	BOOL GetChar(DImage &img, BYTE ch, INT style);

	static BOOL Initialize(VOID);
	static VOID Exit(VOID);

protected:

	struct HEADER {
		DWORD id;				// identifier, always "FONT"
		BYTE begin;				// the first character of font file, always 0x20 (space)
		BYTE end;				// the last character of font file, always 0xff ()
		BYTE width;				// default character width ( == width of space)
		BYTE height;			// default character height ( == height of space)
	};

	struct CHARHEADER {
		BYTE width;				// character width
		BYTE height;			// character height
		BYTE hor_offset;		// horizontal offset of start painting position
		BYTE ver_offset;		// vertical offset of start painting position
	};

	BOOL Load(BUFCPTR data, UINT size);
	BOOL Decode(BYTE ch);
	CONST CHARHEADER *GetCharHeader(BYTE ch) CONST;
	VOID Blit(BUFPTR buf, UINT pitch, INT style);

	static BOOL CheckHeader(CONST HEADER *head, CONST DWORD *char_off, UINT size);
	static BUFPTR LoadFile(HANDLE file, UINT &rd_size, UINT size = 0U);

	IMGCPTR				m_Template;
	BUFPTR				m_DataBuf;
	UINT				m_DataSize;
	CONST HEADER		*m_Header;

	BYTE				m_CurChar;
	BUFPTR				m_CharBuf;
	CONST CHARHEADER	*m_CharHeader;

	static BUFPTR		s_Gid;			// font.gid data buffer

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_FNT_HPP__ */
