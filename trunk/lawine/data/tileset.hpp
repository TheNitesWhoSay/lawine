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
	BOOL Load(INT era);
	VOID Clear(VOID);
	BOOL GetTile(BOOL no_cycling, LTILEIDX index, DImage &img) CONST;
	BOOL GetDoodad(BOOL no_cycling, LTILEIDX index, DImage &img) CONST;
	CONST DPalette *GetPalette(BOOL no_cycling) CONST;
	BOOL GetThumb(LTILEIDX index, MINI_THUMB &thumb) CONST;
	BOOL InitIsoMap(VOID);
	VOID ExitIsoMap(VOID);

//protected:
public:

	static CONST INT CV5_TILE_GROUP_NUM = 1024;
	static CONST INT DOODAD_REF_MAX = 256;
	static CONST INT DOODAD_NUM_MAX = 512;

	struct DDDATA_BIN {
		WORD doodad[DOODAD_REF_MAX];
	};

	class DCoreData;

	BOOL Load(VOID);
	BOOL LoadDddata(HANDLE file, UINT ddnum);

	INT				m_Era;
	DDDATA_BIN		*m_Doodad;
	DCoreData		*m_CcData;
	DCoreData		*m_NcData;

};

/************************************************************************/

class DTileset::DCoreData {

public:

	DCoreData();
	~DCoreData();

	BOOL Load(STRCPTR path);
	VOID Clear(VOID);
	BOOL GenerateThumb(VOID);
	BOOL GetTile(LTILEIDX index, DImage &img) CONST;
	BOOL GetDoodad(LTILEIDX index, DImage &img) CONST;
	CONST DPalette *GetPalette(VOID) CONST;
	UINT GetIsomDict(ISOM_DICT *dict) CONST;
	BOOL GetThumb(LTILEIDX index, MINI_THUMB &thumb) CONST;

protected:

	BOOL LoadCv5(STRCPTR cv5);
	BOOL LoadVf4(STRCPTR vf4);
	BOOL LoadVx4(STRCPTR vx4);
	BOOL LoadVr4(STRCPTR vr4);
	BOOL LoadWpe(STRCPTR wpe);
	BOOL LoadCv5(HANDLE file, UINT tile_num, UINT dd_num);
	BOOL LoadCv5Dd(HANDLE file, UINT dd_num);
	BOOL LoadVf4(HANDLE file, UINT mega_num);
	BOOL LoadVx4(HANDLE file, UINT mega_num);
	BOOL GetMegaTile(BUFPTR buf, UINT pitch, UINT mega_no) CONST;
	BOOL GetMiniTile(BUFPTR buf, UINT pitch, UINT mini_no, BOOL flipped) CONST;
//	DImage *GetDoodad(LTILEIDX index);

protected:

	static CONST INT GROUP_MEGA_NUM = 16;
	static CONST INT MINI_PER_MEGA = 4;
	static CONST INT PIXEL_PER_MINI = 8;

	struct CV5_TILE {
		WORD type;						/* TILE类型ID */
		WORD unknown:4;
		WORD buildable:4;
		WORD ground_height:4;
		WORD unused1:3;
		WORD beacon:1;					/* 是否可以放置Start Location或者Beacon */
		WORD left_abut;					/* 左侧邻接关系ID */
		WORD top_abut;					/* 上方邻接关系ID */
		WORD right_abut;				/* 右侧邻接关系ID */
		WORD bottom_abut;				/* 下方邻接关系ID */
		WORD unused2;
		WORD up_abut;					/* 上层邻接关系ID */
		WORD unused3;
		WORD down_abut;					/* 下层邻接关系ID */
		WORD megatile[GROUP_MEGA_NUM];
	};

	struct CV5_DOODAD {
		WORD type;						/* 对于Doodad总为1 */
		WORD unknown:4;
		WORD buildable:4;
		WORD ground_height:4;
		WORD sprite_overlay:1;			/* 使用Sprites.dat中的图像作为覆盖层 */
		WORD unit_overlay:1;			/* 使用Units.dat中的图像作为覆盖层 */
		WORD overlay_flipped:1;			/* 覆盖层图像左右颠倒存放 */
		WORD beacon:1;					/* 是否可以放置Start Location或者Beacon */
		WORD overlay_id;				/* 覆盖层图像ID（具体含义视sprite_overlay和unit_overlay而定） */
		WORD unused2;
		WORD group_name;				/* 在Staredit中显示的Doodad所属群落的名称（关联到stat_txt.tbl的索引号，需要先减1才能得到正确的索引号） */
		WORD unused3;
		WORD dddata_no;					/* 关联到dddata.bin的索引号 */
		WORD width;						/* Doodad总宽度（TILE数，最大16） */
		WORD height;					/* Doodad总高度（TILE数，高宽之积不能超过256） */
		WORD unused4;
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

	UINT			m_Cv5TileNum;
	UINT			m_Cv5DdNum;
	UINT			m_Vf4Num;
	UINT			m_Vx4Num;
	UINT			m_Vr4Num;
	HANDLE			m_Vr4File;
	BUFPTR			m_Thumb;
	DPalette		m_Palette;
	CV5_TILE		*m_Cv5Tile;
	CV5_DOODAD		*m_Cv5Doodad;
	VF4_MEGATILE	*m_Vf4;
	VX4_MEGATILE	*m_Vx4;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_TILESET_HPP__ */
