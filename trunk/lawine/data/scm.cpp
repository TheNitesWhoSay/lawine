/************************************************************************/
/* File Name   : scm.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : April 11st, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : DScm class implementation                              */
/************************************************************************/

#include "scm.hpp"
#include "../global.hpp"

/************************************************************************/

CONST INT TILE_SIZE = 32;

CONST STRCPTR CHK_FILE_PATH = "staredit\\scenario.chk";

CONST DWORD FOURCC_TYPE = 'EPYT';		// 'TYPE' section
CONST DWORD FOURCC_VER = ' REV';		// 'VER ' section
CONST DWORD FOURCC_ERA = ' ARE';		// 'ERA ' section
CONST DWORD FOURCC_DIM = ' MID';		// 'DIM ' section
CONST DWORD FOURCC_MTXM = 'MXTM';		// 'MTXM' section
CONST DWORD FOURCC_TILE = 'ELIT';		// 'TILE' section
CONST DWORD FOURCC_ISOM = 'MOSI';		// 'ISOM' section

CONST DWORD FOURCC_SWAR = 'SWAR';		// type 'SWAR' for hybrid Starcraft
CONST DWORD FOURCC_BWAR = 'BWAR';		// type 'BWAR' for Brood War

CONST UINT DIM_TINY = 64U;
CONST UINT DIM_SMALL = 96U;
CONST UINT DIM_MEDIUM = 128U;
CONST UINT DIM_LARGE = 192U;
CONST UINT DIM_HUGE = 256U;

CONST UINT MINIMAP_DIM = 128U;

/************************************************************************/

DScm::DScm() :
	m_Valid(FALSE),
	m_Version(L_SCM_VER_UNKNOWN),
	m_Tile(NULL),
	m_Archive(NULL)
{
	m_IsoMap.era = L_ERA_ERROR;
	m_IsoMap.def = 0;
	m_IsoMap.isom = NULL;
	DVarClr(m_IsoMap.size);
}

DScm::~DScm()
{
#if 1
	static BOOL b = FALSE;

	if (!b) {

		DMpq mpq;

		UINT h = 8;
		if (!mpq.CreateArchive("new.mpq", h))
			return;

		CHAR s[] =
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n"
			"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\r\n"
			"cccccccccccccccccccccccccccccccccc\r\n"
			"dddddddddddddddddddddddddddddddddd\r\n"
			"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"
			"ffffffffffffffffffffffffffffffffff\r\n"
			"gggggggggggggggggggggggggggggggggg\r\n"
			"0000000000000000000000000000000000\r\n"
			"__________________________________\r\n"
			"9999999999999999999999999999999999\r\n"
			"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n"
			">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"
			"{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{\r\n"
			"..................................\r\n"
			";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\r\n"
			"**********************************\r\n"
			"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\r\n"
			"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n"
			"5555555555555555555555555555555555\r\n"
			"++++++++++++++++++++++++++++++++++\r\n"
			"//////////////////////////////////\r\n"
			"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\r\n"
			"((((((((((((((((((((((((((((((((((\r\n"
			"----------------------------------\r\n"
			"||||||||||||||||||||||||||||||||||\r\n"
			"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\r\n"
			"3333333333333333333333333333333333\r\n"
			"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n"
			"==================================\r\n"
			"##################################\r\n"
			"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n"
			"))))))))))))))))))))))))))))))))))\r\n"
			"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\r\n"
			"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n"
			"??????????????????????????????????\r\n"
			"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\r\n"
			",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\r\n"
			;

		CHAR f[] = "aaa.txt\r\nok.txt\r\nbbb.txt\r\n1.bmp\r\n";

		if (!mpq.NewFile("bbb.txt", (BUFCPTR)"!}", 2, TRUE, TRUE))
			return;

		if (!mpq.NewFile("aaa.txt", (BUFCPTR)s, strlen(s), TRUE, TRUE))
			return;

		if (!mpq.NewFile("(listfile)", (BUFCPTR)f, strlen(f), TRUE, TRUE))
			return;

		if (!mpq.NewFile("ok.txt", (BUFCPTR)"", 0, TRUE, TRUE))
			return;

		if (!mpq.AddFile("1.bmp", "c:\\x.bmp", TRUE, TRUE))
			return;

		mpq.CloseArchive();

		if (mpq.OpenArchive("new.mpq"))
		{
			CHAR ch[100];
			mpq.ReadFile(mpq.OpenFile("bbb.txt"), ch, 10);
			puts(ch);
		}

		b = TRUE;
	}
#endif

	Clear();
}

/************************************************************************/

BOOL DScm::Create(CONST DTileset &ts, INT def, CONST SIZE &size)
{
	if (m_Valid)
		return FALSE;

	DAssert(!m_IsoMap.isom && !m_Minimap.IsValid());

	if (!DBetween(ts.GetEra(), 0, L_ERA_NUM) || !CheckMapSize(size))
		return FALSE;

	UINT isom_num = CALC_ISOM_ROW(size.cx) * CALC_ISOM_LINE(size.cy);
	LISOMTILE *data = new LISOMTILE[isom_num];
	if (!data)
		return FALSE;

	m_IsoMap.era = ts.GetEra();
	m_IsoMap.def = def;
	m_IsoMap.size = size;
	m_IsoMap.isom = data;

	if (!::NewIsoMap(&m_IsoMap)) {
		Clear();
		return FALSE;
	}

	// TODO: 随机TILE

	if (!MakeMinimap(ts)) {
		Clear();
		return FALSE;
	}

	m_Valid = TRUE;
	return FALSE;
}

BOOL DScm::Load(STRCPTR name)
{
	if (m_Valid)
		return FALSE;

	if (!name)
		return FALSE;

	m_Archive = ::g_Archive.UseArchive(name, 0U);
	if (!m_Archive)
		return FALSE;

	if (!LoadMap()) {
		Clear();
		return FALSE;
	}

	m_Valid = TRUE;
	return TRUE;
}

BOOL DScm::Save(STRCPTR name) CONST
{
	if (!m_Valid)
		return FALSE;

	if (!name)
		return FALSE;

	// TODO:
	return TRUE;
}

VOID DScm::Clear(VOID)
{
	m_Valid = FALSE;

	m_Chk.Clear();
	m_Minimap.Destroy();

	if (m_Archive) {
		::g_Archive.CloseArchive(m_Archive);
		m_Archive = NULL;
	}

	m_IsoMap.era = L_ERA_ERROR;
	m_IsoMap.def = 0;
	DVarClr(m_IsoMap.size);

	delete [] m_IsoMap.isom;
	m_IsoMap.isom = NULL;

	delete [] m_Tile;
	m_Tile = NULL;

	m_Version = L_SCM_VER_UNKNOWN;
}

INT DScm::GetVersion(VOID) CONST
{
	return m_Version;
}

INT DScm::GetEra(VOID) CONST
{
	if (!m_Valid)
		return L_ERA_ERROR;

	return m_IsoMap.era;
}

BOOL DScm::GetSize(SIZE &size) CONST
{
	if (!m_Valid)
		return FALSE;

	size = m_IsoMap.size;
	return TRUE;
}

LTILECPTR DScm::GetTileData(VOID) CONST
{
	if (!m_Valid)
		return NULL;

	return m_Tile;
}

LISOMCPTR DScm::GetIsoMapData(VOID) CONST
{
	if (!m_Valid)
		return NULL;

	return m_IsoMap.isom;
}

IMGCPTR DScm::GetMinimap(VOID) CONST
{
	if (!m_Valid)
		return NULL;

	return m_Minimap;
}

BOOL DScm::GenMinimap(CONST DTileset &ts)
{
	if (!m_Valid || !m_Tile)
		return FALSE;

	if (ts.GetEra() != m_IsoMap.era)
		return FALSE;

	return MakeMinimap(ts);
}

BOOL DScm::IsoBrush(INT brush, CONST POINT &tile_pos)
{
	if (!m_Valid)
		return FALSE;

	return ::BrushIsoMap(&m_IsoMap, brush, &tile_pos);
}

BOOL DScm::UpdateTile(VOID)
{
	if (!m_Valid)
		return FALSE;

	DAssert(m_Tile);

	if (!::GenIsoMapTile(&m_IsoMap, m_Tile))
		return FALSE;

	return TRUE;
}

/************************************************************************/

BOOL DScm::LoadMap(VOID)
{
	if (!m_Chk.Load(CHK_FILE_PATH))
		return FALSE;

	if (!LoadVersion())
		return FALSE;

	if (!LoadEra())
		return FALSE;

	if (!LoadDim())
		return FALSE;
#if 0
	if (!LoadMtxm())
		return FALSE;
#else
	if (!LoadTile())
		return FALSE;

	if (!LoadIsom())
		return FALSE;
#endif

	return TRUE;
}

BOOL DScm::LoadVersion(VOID)
{
	DWORD type;

	m_Version = L_SCM_VER_STARCRAFT;

	UINT size = m_Chk.GetSectionSize(FOURCC_TYPE);
	if (!size)
		return TRUE;

	if (size < sizeof(type))
		return FALSE;

	if (!m_Chk.GetSectionData(FOURCC_TYPE, &type, sizeof(type)))
		return FALSE;

	switch (type) {
	case FOURCC_SWAR:
		m_Version = L_SCM_VER_HYBRID;
		break;
	case FOURCC_BWAR:
		m_Version = L_SCM_VER_BROODWAR;
		break;
	default:
		m_Version = L_SCM_VER_UNKNOWN;
		return FALSE;
	}

	return TRUE;
}

BOOL DScm::LoadEra(VOID)
{
	ERA_DATA era;

	UINT size = m_Chk.GetSectionSize(FOURCC_ERA);
	if (size < sizeof(era))
		return FALSE;

	if (!m_Chk.GetSectionData(FOURCC_ERA, &era, sizeof(era)))
		return FALSE;

	if (era.era >= L_ERA_NUM)
		return FALSE;

	m_IsoMap.era = era.era;
	return TRUE;
}

BOOL DScm::LoadDim(VOID)
{
	DIM_DATA dim;
	SIZE map_size;

	UINT size = m_Chk.GetSectionSize(FOURCC_DIM);
	if (size < sizeof(dim))
		return FALSE;

	if (!m_Chk.GetSectionData(FOURCC_DIM, &dim, sizeof(dim)))
		return FALSE;

	map_size.cx = dim.width;
	map_size.cy = dim.height;
	if (!CheckMapSize(map_size))
		return FALSE;

	m_IsoMap.size = map_size;
	return TRUE;
}

BOOL DScm::LoadMtxm(VOID)
{
	DAssert(!m_Tile);

	UINT map_size = m_IsoMap.size.cx * m_IsoMap.size.cy;
	DAssert(map_size > 0);

	UINT size = m_Chk.GetSectionSize(FOURCC_MTXM);

	m_Tile = new LTILEIDX[map_size];
	if (!m_Tile)
		return FALSE;

	map_size *= sizeof(LTILEIDX);

	// 在星际争霸中如果该段大小大于WxH则忽略多出的部分，如果小于WxH则补0，不会失败
	UINT rd_size = DMin(map_size, size);
	if (rd_size) {
		if(!m_Chk.GetSectionData(FOURCC_MTXM, m_Tile, rd_size))
			return FALSE;
	}

	size = map_size - rd_size;
	if (size)
		DMemClr(m_Tile + rd_size, size);

	// TODO:
	return TRUE;
}

BOOL DScm::LoadTile(VOID)
{
	DAssert(!m_Tile);

	UINT map_size = m_IsoMap.size.cx * m_IsoMap.size.cy;
	DAssert(map_size > 0);

	UINT size = m_Chk.GetSectionSize(FOURCC_TILE);

	m_Tile = new LTILEIDX[map_size];
	if (!m_Tile)
		return FALSE;

	map_size *= sizeof(LTILEIDX);

	// 在星际争霸中如果该段大小大于WxH则忽略多出的部分，如果小于WxH则补0，不会失败
	UINT rd_size = DMin(map_size, size);
	if (rd_size) {
		if(!m_Chk.GetSectionData(FOURCC_TILE, m_Tile, rd_size))
			return FALSE;
	}

	size = map_size - rd_size;
	if (size)
		DMemClr(m_Tile + rd_size, size);

	// TODO:
	return TRUE;
}

BOOL DScm::LoadIsom(VOID)
{
	DAssert(!m_IsoMap.isom);

	UINT size = m_Chk.GetSectionSize(FOURCC_ISOM);
	if (!size)
		return FALSE;

	UINT isom_num = CALC_ISOM_ROW(m_IsoMap.size.cx) * CALC_ISOM_LINE(m_IsoMap.size.cy);
	if (size < isom_num * sizeof(LISOMTILE))
		return FALSE;

	LISOMTILE *isom = new LISOMTILE[isom_num];
	if (!isom)
		return FALSE;

	if (!m_Chk.GetSectionData(FOURCC_ISOM, isom, isom_num * sizeof(LISOMTILE))) {
		delete [] isom;
		return FALSE;
	}

	m_IsoMap.isom = isom;
	return TRUE;
}

BOOL DScm::MakeMinimap(CONST DTileset &ts)
{
	if (!m_Minimap.IsValid()) {
		SIZE size = { MINIMAP_DIM, MINIMAP_DIM };
		if (!m_Minimap.Create(size))
			return FALSE;
	}

	SIZE size = m_IsoMap.size;
	UINT max_dim = DMax(size.cx, size.cy);

	if (max_dim <= DIM_TINY) {
		size.cx *= 2;
		size.cy *= 2;
		MakeSmallMinimap(ts, size);
		return TRUE;
	}

	if (max_dim > DIM_MEDIUM) {
		size.cx /= 2;
		size.cy /= 2;
		MakeLargeMinimap(ts, size);
		return TRUE;
	}

	MakeMediumMinimap(ts, size);
	return TRUE;
}

VOID DScm::MakeSmallMinimap(CONST DTileset &ts, CONST SIZE &size)
{
	DAssert(m_Tile);

	DVerify(m_Minimap.SetSize(size));

	DTileset::MINI_THUMB thumb;
	LTILECPTR tile = m_Tile;
	BUFPTR data = m_Minimap.GetData();
	DAssert(m_Minimap.GetPitch() == MINIMAP_DIM);

	for (INT i = 0; i < m_IsoMap.size.cy; i++) {
		for (INT j = 0; j < m_IsoMap.size.cx; j++, tile++) {
			DVerify(ts.GetThumb(*tile, thumb));
			BUFPTR p = data + j * 2;
			for (INT k = 0; k < 2; k++) {
				DMemCpy(p, thumb.thumb[k], 2 * sizeof(BYTE));
				p += MINIMAP_DIM;
			}
			continue;
		}
		data += MINIMAP_DIM * 2;
	}
}

VOID DScm::MakeMediumMinimap(CONST DTileset &ts, CONST SIZE &size)
{
	DAssert(m_Tile);

	DVerify(m_Minimap.SetSize(size));

	DTileset::MINI_THUMB thumb;
	LTILECPTR tile = m_Tile;
	BUFPTR data = m_Minimap.GetData();
	DAssert(m_Minimap.GetPitch() == MINIMAP_DIM);

	for (INT i = 0; i < size.cy; i++) {
		for (INT j = 0; j < size.cx; j++, tile++) {
			DVerify(ts.GetThumb(*tile, thumb));
			data[j] = thumb.thumb[0][0];
		}
		data += MINIMAP_DIM;
	}
}

VOID DScm::MakeLargeMinimap(CONST DTileset &ts, CONST SIZE &size)
{
	DAssert(m_Tile);

	DVerify(m_Minimap.SetSize(size));

	DTileset::MINI_THUMB thumb;
	LTILECPTR tile = m_Tile;
	BUFPTR data = m_Minimap.GetData();
	DAssert(m_Minimap.GetPitch() == MINIMAP_DIM);

	for (INT i = 0; i < size.cy; i++) {
		for (INT j = 0; j < size.cx; j++, tile += 2) {
			DVerify(ts.GetThumb(*tile, thumb));
			data[j] = thumb.thumb[0][0];
		}
		data += MINIMAP_DIM;
		tile += size.cx * 2;
	}
}

BOOL DScm::CheckMapSize(CONST SIZE &size)
{
	BOOL w_ok = FALSE;
	BOOL h_ok = FALSE;
	UINT dim[] = { DIM_TINY, DIM_SMALL, DIM_MEDIUM, DIM_LARGE, DIM_HUGE };

	for (INT i = 0; i < DCount(dim); i++) {

		if (size.cx == dim[i])
			w_ok = TRUE;

		if (size.cy == dim[i])
			h_ok = TRUE;

		if (w_ok && h_ok)
			return TRUE;
	}

	return FALSE;
}

/************************************************************************/
