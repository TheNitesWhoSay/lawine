/************************************************************************/
/* File Name   : spk.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 15th, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : DSpk class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_SPK_HPP__
#define __SD_LAWINE_DATA_SPK_HPP__

/************************************************************************/

#include <common.h>
#include <image.hpp>

/************************************************************************/

class DSpk {

public:

	DSpk();
	~DSpk();

	BOOL Load(STRCPTR name);
	VOID Clear(VOID);

	INT GetLayerNum(VOID) CONST;
	BOOL GetImage(DImage &img, CONST POINT &view_pos) CONST;
	CONST DImage *GetImage(INT layer) CONST;

protected:

	struct FRAMEHEADER {
		WORD hor_offset;
		WORD ver_offset;
		DWORD bmp_offset;
	};

	struct FRAMEINFO {
		WORD width;
		WORD height;
	};

	BOOL Load(BUFCPTR data, UINT size);
	BOOL LoadLayer(INT layer, WORD frame_num, CONST FRAMEHEADER *frame_head, BUFCPTR data, UINT size);

	static BOOL CheckHeader(BUFCPTR data, UINT size, WORD &layer_num, CONST WORD *&frame_num, CONST FRAMEHEADER *&frame_head);

	INT		m_LayerNum;
	DImage	*m_Layer;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_SPK_HPP__ */
