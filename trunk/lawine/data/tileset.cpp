/************************************************************************/
/* File Name   : tileset.cpp                                            */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 21st, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : DTileset class implementation                          */
/************************************************************************/

#include "tileset.hpp"
#include "../global.hpp"

#ifndef NDEBUG
#include "pcx.hpp"
#endif

/************************************************************************/

static CONST UINT CV5_TILE_GROUP_NUM = 1024U;
static CONST UINT MEGATILE_MAX_NUM = 65536U;		// 0x10000
static CONST UINT MINITILE_MAX_NUM = 32768U;		// 0x08000
static CONST UINT DOODAD_MAX_NUM = 512U;

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
	m_NoCycling(FALSE),
	m_Cv5TileNum(0U),
	m_Cv5DdNum(0U),
	m_Vf4Num(0U),
	m_Vx4Num(0U),
	m_Vr4Num(0U),
	m_Vr4File(NULL),
	m_Cv5Tile(NULL),
	m_Cv5Doodad(NULL),
	m_Vf4(NULL),
	m_Vx4(NULL),
	m_Doodad(NULL),
	m_Thumb(NULL)
{

}

DTileset::~DTileset()
{
	Clear();
}

/************************************************************************/

#ifndef NDEBUG

CAPI RECT __declspec(dllexport) GetCv5(DTileset *ts, UINT grp_no)
{
	RECT r;
	
	r.left = ts->m_Cv5Tile[grp_no].left_abut;
	r.top = ts->m_Cv5Tile[grp_no].top_abut;
	r.right = ts->m_Cv5Tile[grp_no].right_abut;
	r.bottom = ts->m_Cv5Tile[grp_no].bottom_abut;

	return r;
}

CAPI INT __declspec(dllexport) GetCv52(DTileset *ts, UINT grp_no)
{
	return ts->m_Cv5Tile[grp_no].type;
}
CAPI DWORD __declspec(dllexport) GetCv53(DTileset *ts, UINT grp_no)
{
	return MAKELONG(ts->m_Cv5Tile[grp_no].up_abut, ts->m_Cv5Tile[grp_no].down_abut);
}
#endif

INT DTileset::GetEra(VOID) CONST
{
	return m_Era;
}

BOOL DTileset::Load(INT era, BOOL no_cycling)
{
	if (!DBetween(era, 0, L_ERA_NUM))
		return FALSE;

	if (m_Cv5Tile || m_Vr4File)
		return FALSE;

	if (!Load(era, ERA_PATH[era], no_cycling)) {
		Clear();
		return FALSE;
	}

	m_Era = era;
	m_NoCycling = no_cycling;

#ifndef NDEBUG
	DString nstr(ERA_PATH[era]);
	nstr.Format("isom\\%s.txt", (STRCPTR)nstr.Middle(8));
	FILE *fp = fopen(nstr, "w");
	if (!fp)
		return TRUE;
#if 1
	for (int i = 0; i < 1024; i += 2) {
		CV5_TILE *p = &m_Cv5Tile[i];
		fprintf(fp, "[%04X]\n", i);
		fprintf(fp, "type=%04X\nunknown=%X\nbuildable=%X\nground_height=%X\n"
			"unused1=%X\nleft_edge=%04X\ntop_edge=%04X\nright_edge=%04X\nbottom_edge=%04X\n"
			"unused2=%04X\nedge_above=%04X\nunused3=%04X\nedge_below=%04X\n\n",
			p->type, p->unknown, p->buildable, p->ground_height, p->unused1,
			p->left_abut, p->top_abut, p->right_abut, p->bottom_abut,
			p->unused2, p->up_abut, p->unused3, p->down_abut);

		p = &m_Cv5Tile[i + 1];
		fprintf(fp, "[%04X]\n", i + 1);
		fprintf(fp, "type=%04X\nunknown=%X\nbuildable=%X\nground_height=%X\n"
			"unused1=%X\nleft_edge=%04X\ntop_edge=%04X\nright_edge=%04X\nbottom_edge=%04X\n"
			"unused2=%04X\nedge_above=%04X\nunused3=%04X\nedge_below=%04X\n\n",
			p->type, p->unknown, p->buildable, p->ground_height, p->unused1,
			p->left_abut, p->top_abut, p->right_abut, p->bottom_abut,
			p->unused2, p->up_abut, p->unused3, p->down_abut);
#if 1
		if (i > 128)
			continue;

		DImage img;
		IMAGE img2;
		SIZE size = { 32 * 2, 32 * 16 };
		if (!img.Create(size)) {
			continue;
		}
		img2 = *img;
		for (int j = 0; j < 16; j++) {
			LTILEIDX idx = { j, i };
			GetTile(idx, DImage(img2));
			img2.data += 32;
			idx.group_no++;
			GetTile(idx, DImage(img2));
			img2.data += img2.pitch * 32 - 32;
		}
		DString nstr(ERA_PATH[era]);
		nstr.Format("%s\\(%04X)%04X-%04X.pcx", (STRCPTR)nstr.Middle(8), i/2, i, i+1);
		DPcx pcx;
		if (pcx.Create(DImage(*img), m_Palette))
			pcx.Save(nstr);
#endif
	}
#else
	int type = 0;
	int start = 0, end = 0;
	for (int i = 0; i < 1024; i+=2) {

		CV5_TILE *p = &m_Cv5Tile[i];
		if (p->type == type)
			continue;
		if (type) {
			end = i;
			for (int j = start; j < end; j+=2) {
				CV5_TILE *p = &m_Cv5Tile[j];
				fprintf(fp, "\t%04X\t\t\t%02X\t\t", j, p->top_abut);
			}
			fputc('\n', fp);
			for (int j = start; j < end; j+=2) {
				CV5_TILE *p = &m_Cv5Tile[j];
				fprintf(fp, "\t\t\t%02X\t%02X\t%02X\t", p->left_abut, p->type, p->right_abut);
			}
			fputc('\n', fp);
			for (int j = start; j < end; j+=2) {
				CV5_TILE *p = &m_Cv5Tile[j];
				fprintf(fp, "\t\t\t\t%02X\t\t", p->bottom_abut);
			}
			fputc('\n', fp);
		}
		type = p->type;
		start = i;
	}
#endif
	fclose(fp);
#elif 0
	DString nstr(ERA_PATH[era]);
//	nstr.Format("isom\\%s.txt", (STRCPTR)nstr.Middle(8));
//	FILE *fp = fopen(nstr, "w");
	for (int i = 0; i < m_Vf4Num; i++) {
//		fprintf(fp, "[%04X]\n", i);
//		for (int j = 0; j < MINI_PER_MEGA; j++) {
//			for (int k = 0; k < MINI_PER_MEGA; k++) {
//				VF4_MINITILE *p = &m_Vf4[i].minitile[j][k];
//				fprintf(fp, "<%d,%d>\n", k, j);
//				fprintf(fp, "walkable=%X\nlevel=%X\nblock_view=%X\nramp=%X\nunknown=%X\n",
//					p->walkable, p->level, p->block_view, p->ramp, p->unknown);
//			}
//		}
//		fputc('\n', fp);
		DImage img;
		SIZE size = { 32, 32 };
		DVerify(img.Create(size));
		DVerify(GetMegaTile(img.GetData(), img.GetPitch(), i));
		nstr = ERA_PATH[era];
		nstr.Format("%s\\MT\\MT_%04X.pcx", (STRCPTR)nstr.Middle(8), i);
		DPcx pcx;
		if (pcx.Create(img, m_Palette))
			pcx.Save(nstr);
	}
//	fclose(fp);
#endif

#if 0
	for (int i = 0; i < m_Vf4Num; i++) {
		DImage img;
		SIZE size = { 32, 32 };
		DVerify(img.Create(size));
		DVerify(GetMegaTile(img.GetData(), img.GetPitch(), i));
		nstr = ERA_PATH[era];
		nstr.Format("%s\\MT\\MT_%04X.pcx", (STRCPTR)nstr.Middle(8), i);
		DPcx pcx;
		if (pcx.Create(img, m_Palette))
			pcx.Save(nstr);
	}
#endif
	return TRUE;
}

VOID DTileset::Clear(VOID)
{
	delete [] m_Cv5Tile;
	m_Cv5Tile = NULL;

	delete [] m_Cv5Doodad;
	m_Cv5Doodad = NULL;

	delete [] m_Vf4;
	m_Vf4 = NULL;

	delete [] m_Vx4;
	m_Vx4 = NULL;

	delete [] m_Doodad;
	m_Doodad = NULL;

	delete [] m_Thumb;
	m_Thumb = NULL;

	if (m_Vr4File) {
		::g_Archive.CloseFile(m_Vr4File);
		m_Vr4File = NULL;
	}

	m_Cv5TileNum = 0U;
	m_Cv5DdNum = 0U;
	m_Vf4Num = 0U;
	m_Vx4Num = 0U;
	m_Vr4Num = 0U;

	m_Era = L_ERA_NUM;
}

BOOL DTileset::GetTile(LTILEIDX index, DImage &img)
{
	if (!img.IsValid())
		return FALSE;

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

BOOL DTileset::GetDoodad(LTILEIDX index, DImage &img)
{
	if (!img.IsValid())
		return FALSE;

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

BOOL DTileset::InitIsoMap(VOID)
{
	if (!DBetween(m_Era, 0, L_ERA_NUM))
		return FALSE;

	if (!m_Cv5Tile || !m_Cv5TileNum)
		return FALSE;

	UINT num = 0U;
	ISOM_DICT dict[CV5_TILE_GROUP_NUM];

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

	return init_iso_era(m_Era, dict, num);
}

VOID DTileset::ExitIsoMap(VOID)
{
	if (DBetween(m_Era, 0, L_ERA_NUM))
		exit_iso_era(m_Era);
}

CONST DPalette *DTileset::GetPalette(VOID) CONST
{
	if (!m_Cv5Tile)
		return NULL;

	if (!m_Palette.IsValid())
		return NULL;

	return &m_Palette;
}

BOOL DTileset::GetThumb(LTILEIDX index, MINI_THUMB &thumb) CONST
{
	DVarClr(thumb);

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

BOOL DTileset::Load(INT era, STRCPTR name, BOOL no_cycling)
{
	DAssert(name && *name && DBetween(era, 0, L_ERA_NUM));

	DString path(name);

	if (no_cycling) {
		path += "-nc";
		if (!::g_Archive.FileExist(path + ".cv5"))
			path = name;
	}

	if (!LoadCv5(path + ".cv5"))
		return FALSE;

	if (!LoadVf4(path + ".vf4"))
		return FALSE;

	if (!LoadVx4(path + ".vx4"))
		return FALSE;

	if (!LoadVr4(path + ".vr4"))
		return FALSE;

	if (!LoadWpe(path + ".wpe"))
		return FALSE;

	if (!LoadDddata(DString(name) + "\\dddata.bin"))
		return FALSE;

	if (!GenerateThumb())
		return FALSE;

	return TRUE;
}

BOOL DTileset::LoadCv5(STRCPTR cv5)
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

BOOL DTileset::LoadVf4(STRCPTR vf4)
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

BOOL DTileset::LoadVx4(STRCPTR vx4)
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

BOOL DTileset::LoadVr4(STRCPTR vr4)
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

BOOL DTileset::LoadWpe(STRCPTR wpe)
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

BOOL DTileset::LoadDddata(STRCPTR dddata)
{
	DAssert(dddata);

	HANDLE file = ::g_Archive.OpenFile(dddata);
	if (!file)
		return FALSE;

	UINT size = ::g_Archive.GetFileSize(file);
	if (size != DOODAD_MAX_NUM * sizeof(DDDATA_BIN)) {
		::g_Archive.CloseFile(file);
		return FALSE;
	}

	BOOL ret = LoadDddata(file, DOODAD_MAX_NUM);
	::g_Archive.CloseFile(file);
	return ret;
}

BOOL DTileset::GenerateThumb(VOID)
{
	DAssert(!m_Thumb);
	DAssert(m_Vr4File && m_Vr4Num);

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

BOOL DTileset::LoadCv5(HANDLE file, UINT tile_num, UINT dd_num)
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

BOOL DTileset::LoadCv5Dd(HANDLE file, UINT dd_num)
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

BOOL DTileset::LoadVf4(HANDLE file, UINT mega_num)
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

BOOL DTileset::LoadVx4(HANDLE file, UINT mega_num)
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

BOOL DTileset::GetMegaTile(BUFPTR buf, UINT pitch, UINT mega_no)
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

BOOL DTileset::GetMiniTile(BUFPTR buf, UINT pitch, UINT mini_no, BOOL flipped)
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
