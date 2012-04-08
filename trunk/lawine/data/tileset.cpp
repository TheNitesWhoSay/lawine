/************************************************************************/
/* File Name   : tileset.cpp                                            */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 21st, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : DTileset class implementation                          */
/************************************************************************/

#include "tileset.hpp"
#include "../global.hpp"

/************************************************************************/

static CONST UINT MEGATILE_MAX_NUM = 65536U;		// 0x10000
static CONST UINT MINITILE_MAX_NUM = 32768U;		// 0x08000

static CONST STRCPTR ERA_PATH[L_ERA_NUM] = {
	"tileset\\badlands",			// L_ERA_BADLANDS
	"tileset\\platform",			// L_ERA_PLATFORM
	"tileset\\install",				// L_ERA_INSTALL
	"tileset\\ashworld",			// L_ERA_ASHWORLD
	"tileset\\jungle",				// L_ERA_JUNGLE
	"tileset\\desert",				// L_ERA_DESERT
	"tileset\\ice",					// L_ERA_ICE
	"tileset\\twilight",			// L_ERA_TWILIGHT
};

/************************************************************************/

DTileset::DTileset() :
	m_Era(L_ERA_ERROR),
	m_Doodad(NULL),
	m_CcData(NULL),
	m_NcData(NULL)
{

}

DTileset::~DTileset()
{
	Clear();
}

/************************************************************************/

INT DTileset::GetEra(VOID) CONST
{
	return m_Era;
}

BOOL DTileset::Load(INT era)
{
	if (!DBetween(era, 0, L_ERA_NUM))
		return FALSE;

	if (m_CcData || m_NcData)
		return FALSE;

	m_Era = era;

	if (!Load()) {
		Clear();
		return FALSE;
	}

	return TRUE;
}

VOID DTileset::Clear(VOID)
{
	delete m_NcData;
	m_NcData = NULL;

	delete m_CcData;
	m_CcData = NULL;

	delete [] m_Doodad;
	m_Doodad = NULL;

	m_Era = L_ERA_NUM;
}

BOOL DTileset::GetTile(BOOL no_cycling, LTILEIDX index, DImage &img) CONST
{
	if (!img.IsValid())
		return FALSE;

	if (no_cycling && m_NcData)
		return m_NcData->GetTile(index, img);

	if (m_CcData)
		return m_CcData->GetTile(index, img);

	return FALSE;
}

BOOL DTileset::GetDoodad(BOOL no_cycling, LTILEIDX index, DImage &img) CONST
{
	if (!img.IsValid())
		return FALSE;

	if (no_cycling && m_NcData)
		return m_NcData->GetDoodad(index, img);

	if (m_CcData)
		return m_CcData->GetDoodad(index, img);

	return FALSE;
}

CONST DPalette *DTileset::GetPalette(BOOL no_cycling) CONST
{
	if (no_cycling && m_NcData)
		return m_NcData->GetPalette();

	if (m_CcData)
		return m_CcData->GetPalette();

	return NULL;
}

BOOL DTileset::GetThumb(LTILEIDX index, MINI_THUMB &thumb) CONST
{
	DVarClr(thumb);

	// 小地图总是使用无调色板动画的图像数据
	if (m_NcData)
		return m_NcData->GetThumb(index, thumb);

	if (m_CcData)
		return m_CcData->GetThumb(index, thumb);

	return FALSE;
}

BOOL DTileset::InitIsoMap(VOID)
{
	if (!DBetween(m_Era, 0, L_ERA_NUM))
		return FALSE;

	if (!m_CcData)
		return FALSE;

	ISOM_DICT dict[CV5_TILE_GROUP_NUM];
	DVarClr(dict);

	UINT num = m_CcData->GetIsomDict(dict);
	if (!num)
		return FALSE;

	return init_iso_era(m_Era, dict, num);
}

VOID DTileset::ExitIsoMap(VOID)
{
	if (DBetween(m_Era, 0, L_ERA_NUM))
		exit_iso_era(m_Era);
}

/************************************************************************/

BOOL DTileset::Load(VOID)
{
	DAssert(DBetween(m_Era, 0, L_ERA_NUM));

	DString path(ERA_PATH[m_Era]);

	m_CcData = new DCoreData;
	if (!m_CcData->Load(path)) {
		m_CcData->Clear();
		return FALSE;
	}

	m_NcData = new DCoreData;
	if (!m_NcData->Load(path + "-nc")) {
		delete m_NcData;
		m_NcData = NULL;
		if (!m_CcData->GenerateThumb())
			return FALSE;
	} else {
		if (!m_NcData->GenerateThumb())
			return FALSE;
	}

	HANDLE file = ::g_Archive.OpenFile(path + "\\dddata.bin");
	if (!file)
		return FALSE;

	BOOL ret = FALSE;
	UINT size = ::g_Archive.GetFileSize(file);
	if (size == DOODAD_NUM_MAX * sizeof(DDDATA_BIN))
		ret = LoadDddata(file, DOODAD_NUM_MAX);

	::g_Archive.CloseFile(file);
	return ret;
}

BOOL DTileset::LoadDddata(HANDLE file, UINT ddnum)
{
	DAssert(file && ddnum);
	DAssert(!m_Doodad);

	DDDATA_BIN *dddata = new DDDATA_BIN[ddnum];
	UINT size = ddnum * sizeof(DDDATA_BIN);

	if (::g_Archive.ReadFile(file, dddata, size) != size) {
		delete [] dddata;
		return FALSE;
	}

	m_Doodad = dddata;
	return TRUE;
}

/************************************************************************/

DTileset::DCoreData::DCoreData() :
	m_Cv5TileNum(0),
	m_Cv5DdNum(0),
	m_Vf4Num(0),
	m_Vx4Num(0),
	m_Vr4Num(0),
	m_Thumb(NULL),
	m_Vr4File(NULL),
	m_Cv5Tile(NULL),
	m_Cv5Doodad(NULL),
	m_Vf4(NULL),
	m_Vx4(NULL)
{

}

DTileset::DCoreData::~DCoreData()
{
	Clear();
}

BOOL DTileset::DCoreData::Load(STRCPTR path)
{
	DString str(path);

	if (!LoadCv5(str + ".cv5"))
		return FALSE;

	if (!LoadVf4(str + ".vf4"))
		return FALSE;

	if (!LoadVx4(str + ".vx4"))
		return FALSE;

	if (!LoadVr4(str + ".vr4"))
		return FALSE;

	if (!LoadWpe(str + ".wpe"))
		return FALSE;

	return TRUE;
}

VOID DTileset::DCoreData::Clear(VOID)
{
	m_Cv5TileNum = 0U;
	m_Cv5DdNum = 0U;
	m_Vf4Num = 0U;
	m_Vx4Num = 0U;
	m_Vr4Num = 0U;

	delete [] m_Thumb;
	m_Thumb = NULL;

	delete [] m_Cv5Tile;
	m_Cv5Tile = NULL;

	delete [] m_Cv5Doodad;
	m_Cv5Doodad = NULL;

	delete [] m_Vf4;
	m_Vf4 = NULL;

	delete [] m_Vx4;
	m_Vx4 = NULL;

	if (m_Vr4File) {
		::g_Archive.CloseFile(m_Vr4File);
		m_Vr4File = NULL;
	}
}

BOOL DTileset::DCoreData::GenerateThumb(VOID)
{
	if (m_Thumb)
		return TRUE;

	if (!m_Vr4File || !m_Vr4Num)
		return FALSE;

	if (::g_Archive.SeekFile(m_Vr4File, 0) == ERROR_POS)
		return FALSE;

	m_Thumb = new BYTE[m_Vr4Num];

	for (UINT mini_no = 0; mini_no < m_Vr4Num; mini_no++) {

		VR4_MINITILE mini_tile;
		if (::g_Archive.ReadFile(m_Vr4File, &mini_tile, sizeof(mini_tile)) != sizeof(mini_tile))
			return FALSE;

		m_Thumb[mini_no] = mini_tile.bitmap[6][7];
	}

	return TRUE;
}

BOOL DTileset::DCoreData::GetTile(LTILEIDX index, DImage &img) CONST
{
	DAssert(img.IsValid());

	if (index.group_no >= m_Cv5TileNum)
		return GetDoodad(index, img);

	if (!m_Cv5TileNum || !m_Cv5Tile)
		return FALSE;

	if (img.GetWidth() < L_TILE_SIZE || img.GetHeight() < L_TILE_SIZE)
		return FALSE;

	DAssert(img.GetPitch() >= L_TILE_SIZE);

	UINT mega_no = m_Cv5Tile[index.group_no].megatile[index.mega_index];
	return GetMegaTile(img.GetData(), img.GetPitch(), mega_no);
}

BOOL DTileset::DCoreData::GetDoodad(LTILEIDX index, DImage &img) CONST
{
	DAssert(img.IsValid());

	if (!m_Cv5DdNum || !m_Cv5Doodad)
		return FALSE;

	if (index.group_no < m_Cv5TileNum)
		return FALSE;

	UINT ddg_no = index.group_no - m_Cv5TileNum;
	if (ddg_no >= m_Cv5DdNum)
		return FALSE;

	if (img.GetWidth() < L_TILE_SIZE || img.GetHeight() < L_TILE_SIZE)
		return FALSE;

	DAssert(img.GetPitch() >= L_TILE_SIZE);

	UINT mega_no = m_Cv5Doodad[ddg_no].megatile[index.mega_index];

	return GetMegaTile(img.GetData(), img.GetPitch(), mega_no);
}

CONST DPalette *DTileset::DCoreData::GetPalette(VOID) CONST
{
	if (!m_Cv5Tile)
		return NULL;

	if (!m_Palette.IsValid())
		return NULL;

	return &m_Palette;
}

UINT DTileset::DCoreData::GetIsomDict(ISOM_DICT *dict) CONST
{
	UINT num = 0U;

	for (UINT i = 0; i < m_Cv5TileNum; i++) {

		CV5_TILE *cv5 = &m_Cv5Tile[i];
		if (!cv5->type)
			continue;

		dict[num].group_no = i;
		dict[num].tile.type = cv5->type;
		dict[num].tile.left_abut = cv5->left_abut;
		dict[num].tile.top_abut = cv5->top_abut;
		dict[num].tile.right_abut = cv5->right_abut;
		dict[num].tile.bottom_abut = cv5->bottom_abut;
		dict[num].tile.up_abut = cv5->up_abut;
		dict[num].tile.down_abut = cv5->down_abut;

		WORD mega_mask = 0;
		for (INT j = 0; j < GROUP_MEGA_NUM; j++) {
			if (cv5->megatile[j])
				mega_mask |= 1 << j;
		}

		dict[num++].mega_mask = mega_mask;
	}

	return num;
}

BOOL DTileset::DCoreData::GetThumb(LTILEIDX index, MINI_THUMB &thumb) CONST
{
	if (!m_Thumb || !m_Cv5Tile || !m_Vx4 || !m_Vr4Num)
		return FALSE;

	UINT mega_no = 0U;
	if (index.group_no < m_Cv5TileNum) {
		mega_no = m_Cv5Tile[index.group_no].megatile[index.mega_index];
	} else {
		UINT ddg_no = index.group_no - m_Cv5TileNum;
		if (ddg_no >= m_Cv5DdNum)
			return FALSE;
		mega_no = m_Cv5Doodad[ddg_no].megatile[index.mega_index];
	}

	if (mega_no >= m_Vx4Num)
		return FALSE;

	for (INT i = 0; i < THUMB_PER_MEGA; i++) {
		for (INT j = 0; j < THUMB_PER_MEGA; j++) {
			UINT mini_no = m_Vx4[mega_no].minitile[i][j].graphics;
			if (mini_no < m_Vr4Num)
				thumb.thumb[i][j] = m_Thumb[mini_no];
		}
	}

	return TRUE;
}

/************************************************************************/

BOOL DTileset::DCoreData::LoadCv5(STRCPTR cv5)
{
	DAssert(cv5);

	HANDLE file = ::g_Archive.OpenFile(cv5);
	if (!file)
		return FALSE;

	DAssert(sizeof(CV5_TILE) == sizeof(CV5_DOODAD));

	UINT size = ::g_Archive.GetFileSize(file);
	if (size <= CV5_TILE_GROUP_NUM * sizeof(CV5_TILE)) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	size -= CV5_TILE_GROUP_NUM * sizeof(CV5_TILE);
	if (size % sizeof(CV5_DOODAD)) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	UINT dd_num = size / sizeof(CV5_DOODAD);

	BOOL ret = LoadCv5(file, CV5_TILE_GROUP_NUM, dd_num);
	::g_Archive.CloseFile(file);
	return ret;
}

BOOL DTileset::DCoreData::LoadVf4(STRCPTR vf4)
{
	DAssert(vf4);

	HANDLE file = ::g_Archive.OpenFile(vf4);
	if (!file)
		return FALSE;

	UINT size = ::g_Archive.GetFileSize(file);
	if (!size || size % sizeof(VF4_MEGATILE)) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	UINT mega_num = size / sizeof(VF4_MEGATILE);
	if (mega_num > MEGATILE_MAX_NUM) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	BOOL ret = LoadVf4(file, mega_num);
	::g_Archive.CloseFile(file);
	return ret;
}

BOOL DTileset::DCoreData::LoadVx4(STRCPTR vx4)
{
	DAssert(vx4);

	HANDLE file = ::g_Archive.OpenFile(vx4);
	if (!file)
		return FALSE;

	UINT size = ::g_Archive.GetFileSize(file);
	if (!size || size % sizeof(VX4_MEGATILE)) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	UINT mega_num = size / sizeof(VX4_MEGATILE);
	if (mega_num > MEGATILE_MAX_NUM) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	BOOL ret = LoadVx4(file, mega_num);
	::g_Archive.CloseFile(file);
	return ret;
}

BOOL DTileset::DCoreData::LoadVr4(STRCPTR vr4)
{
	DAssert(vr4);

	HANDLE file = ::g_Archive.OpenFile(vr4);
	if (!file)
		return FALSE;

	UINT size = ::g_Archive.GetFileSize(file);
	if (!size || size % sizeof(VR4_MINITILE)) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	m_Vr4Num = size / sizeof(VR4_MINITILE);
	if (m_Vr4Num > MINITILE_MAX_NUM)
		return FALSE;

	m_Vr4File = file;
	return TRUE;
}

BOOL DTileset::DCoreData::LoadWpe(STRCPTR wpe)
{
	DAssert(wpe);

	HANDLE file = ::g_Archive.OpenFile(wpe);
	if (!file)
		return FALSE;

	UINT size = ::g_Archive.GetFileSize(file);
	if (size != D_COLOR_NUM * sizeof(WPE_COLOR)) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	DAssert(size == D_COLOR_NUM * sizeof(COLOR));

	BOOL ret = FALSE;
	if (::g_Archive.ReadFile(file, m_Palette, size) == size)
		ret = TRUE;

	::g_Archive.CloseFile(file);
	return ret;
}

BOOL DTileset::DCoreData::LoadCv5(HANDLE file, UINT tile_num, UINT dd_num)
{
	DAssert(file && tile_num);

	CV5_TILE *cv5_tile = new CV5_TILE[tile_num];
	UINT size = tile_num * sizeof(CV5_TILE);
	if (::g_Archive.ReadFile(file, cv5_tile, size) != size) {
		delete [] cv5_tile;
		return FALSE;
	}

	if (!LoadCv5Dd(file, dd_num)) {
		delete [] cv5_tile;
		return FALSE;
	}

	m_Cv5Tile = cv5_tile;
	m_Cv5TileNum = tile_num;
	return TRUE;
}

BOOL DTileset::DCoreData::LoadCv5Dd(HANDLE file, UINT dd_num)
{
	DAssert(file);

	if (!dd_num) {
		m_Cv5DdNum = 0U;
		return TRUE;
	}

	CV5_DOODAD *cv5_dd = new CV5_DOODAD[dd_num];
	UINT size = dd_num * sizeof(CV5_DOODAD);
	if (::g_Archive.ReadFile(file, cv5_dd, size) != size) {
		delete [] cv5_dd;
		return FALSE;
	}

	m_Cv5Doodad = cv5_dd;
	m_Cv5DdNum = dd_num;
	return TRUE;
}

BOOL DTileset::DCoreData::LoadVf4(HANDLE file, UINT mega_num)
{
	DAssert(file && mega_num);

	VF4_MEGATILE *vf4 = new VF4_MEGATILE[mega_num];
	UINT size = mega_num * sizeof(VF4_MEGATILE);
	if (::g_Archive.ReadFile(file, vf4, size) != size) {
		delete [] vf4;
		return FALSE;
	}

	m_Vf4 = vf4;
	m_Vf4Num = mega_num;
	return TRUE;
}

BOOL DTileset::DCoreData::LoadVx4(HANDLE file, UINT mega_num)
{
	DAssert(file && mega_num);

	VX4_MEGATILE *vx4 = new VX4_MEGATILE[mega_num];
	UINT size = mega_num * sizeof(VX4_MEGATILE);
	if (::g_Archive.ReadFile(file, vx4, size) != size) {
		delete [] vx4;
		return FALSE;
	}

	m_Vx4 = vx4;
	m_Vx4Num = mega_num;
	return TRUE;
}

BOOL DTileset::DCoreData::GetMegaTile(BUFPTR buf, UINT pitch, UINT mega_no) CONST
{
	DAssert(buf && pitch >= L_TILE_SIZE);

	if (mega_no >= m_Vx4Num)
		return FALSE;

	CONST VX4_MEGATILE *vx4 = m_Vx4 + mega_no;

	for (INT i = 0; i < MINI_PER_MEGA; i++) {
		BUFPTR p = buf;
		for (INT j = 0; j < MINI_PER_MEGA; j++) {
			CONST VX4_MINITILE *mini = &vx4->minitile[i][j];
			DVerify(GetMiniTile(p, pitch, mini->graphics, mini->flipped));
			p += PIXEL_PER_MINI;
		}
		buf += pitch * PIXEL_PER_MINI;
	}

	return TRUE;
}

BOOL DTileset::DCoreData::GetMiniTile(BUFPTR buf, UINT pitch, UINT mini_no, BOOL flipped) CONST
{
	DAssert(buf && pitch >= PIXEL_PER_MINI && m_Vr4File);

	if (mini_no >= m_Vr4Num)
		return FALSE;

	UINT size = sizeof(VR4_MINITILE);
	if (::g_Archive.SeekFile(m_Vr4File, mini_no * size) == ERROR_POS)
		return FALSE;

	VR4_MINITILE vr4;
	if (::g_Archive.ReadFile(m_Vr4File, vr4.bitmap, size) != size)
		return FALSE;

	if (!flipped) {
		for (INT i = 0; i < PIXEL_PER_MINI; i++) {
			DMemCpy(buf, vr4.bitmap[i], PIXEL_PER_MINI);
			buf += pitch;
		}
	} else {
		for (INT i = 0; i < PIXEL_PER_MINI; i++) {
			for (INT j = 0; j < PIXEL_PER_MINI; j++)
				buf[j] = vr4.bitmap[i][PIXEL_PER_MINI - j - 1];
			buf += pitch;
		}
	}

	return TRUE;
}

/************************************************************************/
