/************************************************************************/
/* File Name   : tileset.hpp                                            */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 21st, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : DTileset class declaration                             */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_TILESET_HPP__
#define __SD_LAWINE_DATA_TILESET_HPP__

/************************************************************************/

#include <common.h>
#include <lawinedef.h>
#include <image.hpp>
#include <palette.hpp>
#include <string.hpp>
#include "../misc/isomap.h"

/************************************************************************/

class DTileset {

public:

	static CONST INT THUMB_PER_MEGA = 2;

	struct MINI_THUMB {
		BYTE thumb[THUMB_PER_MEGA][THUMB_PER_MEGA];
	};

	DTileset();
	~DTileset();

	INT GetEra(VOID) CONST;
	BOOL Load(INT era, BOOL no_cycling);
	VOID Clear(VOID);
	BOOL GetTile(LTILEIDX index, DImage &img);
	BOOL GetDoodad(LTILEIDX index, DImage &img);
	BOOL InitIsoMap(VOID);
	VOID ExitIsoMap(VOID);
	CONST DPalette *GetPalette(VOID) CONST;

	BOOL GetThumb(LTILEIDX index, MINI_THUMB &thumb) CONST;

//protected:
public:

	static CONST INT GROUP_MEGA_NUM = 16;
	static CONST INT MINI_PER_MEGA = 4;
	static CONST INT PIXEL_PER_MINI = 8;
	static CONST INT DOODAD_REF_MAX = 256;

	struct CV5_TILE {
		WORD type;
		WORD unknown:4;
		WORD buildable:4;
		WORD ground_height:4;
		WORD unused1:4;
		WORD left_abut;
		WORD top_abut;
		WORD right_abut;
		WORD bottom_abut;
		WORD unused2;
		WORD up_abut;
		WORD unused3;
		WORD down_abut;
		WORD megatile[GROUP_MEGA_NUM];
	};

	struct CV5_DOODAD {
		WORD type;
		WORD unknown:4;
		WORD buildable:4;
		WORD ground_height:4;
		WORD overlay_attr:4;
		WORD overlay_id;
		WORD unused1;
		WORD string_no;
		WORD unused2;
		WORD dddata_no;
		WORD width;
		WORD height;
		WORD unused3;
		WORD megatile[GROUP_MEGA_NUM];
	};

	struct VF4_MINITILE {
		WORD walkable:1;
		WORD level:2;
		WORD block_view:1;
		WORD ramp:1;
		WORD unknown:11;
	};

	struct VF4_MEGATILE {
		VF4_MINITILE minitile[MINI_PER_MEGA][MINI_PER_MEGA];
	};

	struct VX4_MINITILE {
		WORD flipped:1;
		WORD graphics:15;
	};

	struct VX4_MEGATILE {
		VX4_MINITILE minitile[MINI_PER_MEGA][MINI_PER_MEGA];
	};

	struct VR4_MINITILE {
		BYTE bitmap[PIXEL_PER_MINI][PIXEL_PER_MINI];
	};

	struct WPE_COLOR {
		BYTE red;
		BYTE green;
		BYTE blue;
		BYTE reserved;
	};

	struct DDDATA_BIN {
		WORD doodad[DOODAD_REF_MAX];
	};

	BOOL Load(INT era, STRCPTR name, BOOL no_cycling);
	BOOL LoadCv5(STRCPTR cv5);
	BOOL LoadVf4(STRCPTR vf4);
	BOOL LoadVx4(STRCPTR vx4);
	BOOL LoadVr4(STRCPTR vr4);
	BOOL LoadWpe(STRCPTR wpe);
	BOOL LoadDddata(STRCPTR dddata);
	BOOL GenerateThumb(VOID);
	BOOL LoadCv5(HANDLE file, UINT tile_num, UINT dd_num);
	BOOL LoadCv5Dd(HANDLE file, UINT dd_num);
	BOOL LoadVf4(HANDLE file, UINT mega_num);
	BOOL LoadVx4(HANDLE file, UINT mega_num);
	BOOL LoadDddata(HANDLE file, UINT ddnum);
	BOOL GetMegaTile(BUFPTR buf, UINT pitch, UINT mega_no);
	BOOL GetMiniTile(BUFPTR buf, UINT pitch, UINT mini_no, BOOL flipped);
//	DImage *GetDoodad(LTILEIDX index);

	INT				m_Era;
	BOOL			m_NoCycling;
	DPalette		m_Palette;
	UINT			m_Cv5TileNum;
	UINT			m_Cv5DdNum;
	UINT			m_Vf4Num;
	UINT			m_Vx4Num;
	UINT			m_Vr4Num;
	HANDLE			m_Vr4File;
	BUFPTR			m_Thumb;
	CV5_TILE		*m_Cv5Tile;
	CV5_DOODAD		*m_Cv5Doodad;
	VF4_MEGATILE	*m_Vf4;
	VX4_MEGATILE	*m_Vx4;
	DDDATA_BIN		*m_Doodad;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_TILESET_HPP__ */
