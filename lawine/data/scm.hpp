/************************************************************************/
/* File Name   : scm.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : April 11st, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : DScm class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_SCM_HPP__
#define __SD_LAWINE_DATA_SCM_HPP__

/************************************************************************/

#include <common.h>
#include <image.hpp>
#include <lawinedef.h>
#include "chk.hpp"
#include "mpq.hpp"
#include "tileset.hpp"
#include "../misc/isomap.h"

/************************************************************************/

class DScm {

protected:

	struct ERA_DATA {
		WORD era:3;
		WORD unused:13;
	};

	struct DIM_DATA {
		WORD width;
		WORD height;
	};

public:

	DScm();
	~DScm();

	BOOL Create(CONST DTileset &ts, INT def, CONST SIZE &size);
	BOOL Load(STRCPTR name);
	BOOL Save(STRCPTR name) CONST;
	VOID Clear(VOID);

	INT GetVersion(VOID) CONST;
	INT GetEra(VOID) CONST;
	BOOL GetSize(SIZE &size) CONST;
	LTILECPTR GetTileData(VOID) CONST;
	LISOMCPTR GetIsoMapData(VOID) CONST;

	IMGCPTR GetMinimap(VOID) CONST;
	BOOL GenMinimap(CONST DTileset &ts);

	BOOL IsoBrush(INT brush, CONST POINT &tile_pos);
	BOOL UpdateTile(VOID);

protected:

	BOOL LoadMap(VOID);
	BOOL LoadVersion(VOID);
	BOOL LoadEra(VOID);
	BOOL LoadDim(VOID);
	BOOL LoadMtxm(VOID);
	BOOL LoadTile(VOID);
	BOOL LoadIsom(VOID);

	BOOL MakeMinimap(CONST DTileset &ts);
	VOID MakeSmallMinimap(CONST DTileset &ts, CONST SIZE &size);
	VOID MakeMediumMinimap(CONST DTileset &ts, CONST SIZE &size);
	VOID MakeLargeMinimap(CONST DTileset &ts, CONST SIZE &size);

	static BOOL CheckMapSize(CONST SIZE &size);

	BOOL			m_Valid;
	INT				m_Version;
	LTILEPTR		m_Tile;
	ISOM_MAP		m_IsoMap;
	DChk			m_Chk;
	DImage			m_Minimap;
	DMpq			*m_Archive;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_SCM_HPP__ */
