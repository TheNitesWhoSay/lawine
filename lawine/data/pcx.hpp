/************************************************************************/
/* File Name   : pcx.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 21st, 2008                                         */
/* Module      : Lawine library                                         */
/* Descript    : DPcx class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_PCX_HPP__
#define __SD_LAWINE_DATA_PCX_HPP__

/************************************************************************/

#include <common.h>
#include <image.hpp>
#include <palette.hpp>
#include <file.hpp>

/************************************************************************/

class DPcx {

protected:

	struct HEADER {
		BYTE manufacturer;
		BYTE version;
		BYTE encoding;
		BYTE depth;
		WORD xmin;
		WORD ymin;
		WORD xmax;
		WORD ymax;
		WORD hor_dpi;
		WORD ver_dpi;
		BYTE palette[48];
		BYTE reserved;
		BYTE planes;
		WORD pitch;
		WORD pal_info;
		WORD hor_scr_size;
		WORD ver_scr_size;
		BYTE filler[54];
	};

	struct COLOR {
		BYTE red;
		BYTE green;
		BYTE blue;
	};

	struct PALETTE {
		BYTE flag;
		COLOR colors[256];
	};

public:

	DPcx();
	~DPcx();

	BOOL Create(CONST DImage &img, CONST DPalette &pal);
	BOOL Load(STRCPTR name);
	BOOL Save(STRCPTR name) CONST;
	VOID Clear(VOID);
	BOOL Decode(VOID);
	CONST DImage *GetImage(VOID) CONST;
	CONST DPalette *GetPalette(VOID) CONST;

protected:

	BOOL CheckHeader(CONST HEADER *&head, BUFCPTR &img, CONST PALETTE *&pal) CONST;
	BOOL Encode(DFile &file, BUFPTR buf, UINT buf_size) CONST;

	static UINT EncodeRLE(BUFCPTR src, UINT src_size, BUFPTR dest, UINT dest_size);
	static UINT DecodeRLE(BUFCPTR src, UINT src_size, BUFPTR dest, UINT dest_size);

	BUFPTR		m_DataBuf;
	UINT		m_DataSize;
	DImage		m_Image;
	DPalette	m_Palette;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_PCX_HPP__ */
