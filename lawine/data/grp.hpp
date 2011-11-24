/************************************************************************/
/* File Name   : grp.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 6th, 2009                                          */
/* Module      : Lawine library                                         */
/* Descript    : DGrp class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_GRP_HPP__
#define __SD_LAWINE_DATA_GRP_HPP__

/************************************************************************/

#include <common.h>
#include <image.hpp>

/************************************************************************/

class DGrp {

public:

	DGrp();
	~DGrp();

	BOOL Load(STRCPTR name);
	VOID Clear(VOID);
	BOOL Decode(INT frame_no);
	INT GetFrameNum(VOID) CONST;
	CONST DImage *GetImage(VOID) CONST;

protected:

	struct HEADER {
		WORD frame_num;
		WORD width;
		WORD height;
	};

	struct FRAMEHEADER {
		BYTE hor_offset;
		BYTE ver_offset;
		BYTE pixel_per_line;
		BYTE line_num;
		DWORD line_offset;
	};

	class DFrame;

	BOOL Load(BUFCPTR data, UINT size);
	DFrame *LoadFrame(CONST FRAMEHEADER *frame_head, UINT pitch);

	static BOOL CheckHeader(BUFCPTR data, UINT size, CONST HEADER *&head, CONST FRAMEHEADER *&frame_head);

	DFrame			**m_FrameList;
	INT				m_FrameNum;
	INT				m_CacheFrame;
	BUFCPTR			m_DataBuf;
	UINT			m_DataSize;
	DImage			m_Image;

};

/************************************************************************/

class DGrp::DFrame {

public:
	
	DFrame(BUFPTR buf, UINT pitch);
	~DFrame();

	BOOL Load(CONST FRAMEHEADER *head, BUFCPTR data);
	VOID Clear(VOID);
	VOID Decode(VOID);

protected:

	typedef WORD LINEOFFSET;

	static BOOL CheckHeader(CONST FRAMEHEADER *head);
	static VOID DecodeLine(BUFCPTR line_begin, BUFCPTR line_end, BUFPTR buf);

	BUFPTR				m_Buffer;
	UINT				m_Pitch;
	CONST FRAMEHEADER	*m_Header;
	CONST LINEOFFSET	*m_LineOffset;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_GRP_HPP__ */
