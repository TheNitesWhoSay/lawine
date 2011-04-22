/************************************************************************/
/* File Name   : scm.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : April 11st, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : DScm class implementation                              */
/************************************************************************/

#include <array.hpp>
#include "scm.hpp"
#include "../global.hpp"

/************************************************************************/

#define SAFE_ROL(x, n)	(((x) << ((n) & 0x1f)) | (x >> (32 - ((n) & 0x1f))))
#define SAFE_ROR(x, n)	(((x) >> ((n) & 0x1f)) | (x << (32 - ((n) & 0x1f))))

/************************************************************************/

CONST DWORD FOURCC_TYPE = 'EPYT';		// 'TYPE' section
CONST DWORD FOURCC_VER = ' REV';		// 'VER ' section
CONST DWORD FOURCC_IVE2 = '2EVI';		// 'IVE2' section
CONST DWORD FOURCC_VCOD = 'DOCV';		// 'VCOD' section
CONST DWORD FOURCC_IOWN = 'NWOI';		// 'IOWN' section
CONST DWORD FOURCC_OWNR = 'RNWO';		// 'OWNR' section
CONST DWORD FOURCC_ERA = ' ARE';		// 'ERA ' section
CONST DWORD FOURCC_DIM = ' MID';		// 'DIM ' section
CONST DWORD FOURCC_SIDE = 'EDIS';		// 'SIDE' section
CONST DWORD FOURCC_MTXM = 'MXTM';		// 'MTXM' section
CONST DWORD FOURCC_PUNI = 'INUP';		// 'PUNI' section
CONST DWORD FOURCC_UNIT = 'TINU';		// 'UNIT' section
CONST DWORD FOURCC_ISOM = 'MOSI';		// 'ISOM' section
CONST DWORD FOURCC_TILE = 'ELIT';		// 'TILE' section
CONST DWORD FOURCC_DD2 = ' 2DD';		// 'DD2 ' section
CONST DWORD FOURCC_THG2 = '2GHT';		// 'THG2' section
CONST DWORD FOURCC_MASK = 'KSAM';		// 'MASK' section
CONST DWORD FOURCC_STR = ' RTS';		// 'STR ' section
CONST DWORD FOURCC_UPRP = 'PRPU';		// 'UPRP' section
CONST DWORD FOURCC_UPUS = 'SUPU';		// 'UPUS' section
CONST DWORD FOURCC_MRGN = 'NGRM';		// 'MRGN' section
CONST DWORD FOURCC_TRIG = 'GIRT';		// 'TRIG' section
CONST DWORD FOURCC_MBRF = 'FRBM';		// 'MBRF' section
CONST DWORD FOURCC_SPRP = 'PRPS';		// 'SPRP' section
CONST DWORD FOURCC_FORC = 'CROF';		// 'FORC' section
CONST DWORD FOURCC_WAV = ' VAW';		// 'WAV ' section
CONST DWORD FOURCC_SWNM = 'MNWS';		// 'SWNM' section
CONST DWORD FOURCC_COLR = 'RLOC';		// 'CLOR' section
CONST DWORD FOURCC_PUPx = 'xPUP';		// 'PUPx' section
CONST DWORD FOURCC_PTEx = 'xETP';		// 'PTEx' section
CONST DWORD FOURCC_UNIx = 'xINU';		// 'UNIx' section
CONST DWORD FOURCC_UPGx = 'xGPU';		// 'UPGx' section
CONST DWORD FOURCC_TECx = 'xCET';		// 'TECx' section

CONST DWORD FOURCC_SWAR = 'SWAR';		// type 'SWAR' for hybrid Starcraft
CONST DWORD FOURCC_BWAR = 'BWAR';		// type 'BWAR' for Brood War

CONST UINT DIM_TINY = 64U;
CONST UINT DIM_SMALL = 96U;
CONST UINT DIM_MEDIUM = 128U;
CONST UINT DIM_LARGE = 192U;
CONST UINT DIM_HUGE = 256U;

CONST UINT MINIMAP_DIM = 128U;

CONST STRCPTR CHK_FILE_PATH = "staredit\\scenario.chk";

/************************************************************************/

CONST DScm::VCODE DScm::VERIFY_CODE = {
	{
		0x77ca1934, 0x7168dc99, 0xc3bf600a, 0xa775e7a7, 0xa67d291f, 0xbb3ab0d7, 0xed2431cc, 0x0b134c17,
		0xb7a22065, 0x6b18bd91, 0xdd5dc38d, 0x37d57ae2, 0xd46459f6, 0x0f129a63, 0x462e5c43, 0x2af874e3,
		0x06376a08, 0x3bd6f637, 0x1663940e, 0xec5c6745, 0xb7f77bd7, 0x9ed4fc1a, 0x8c3ffa73, 0x0fe1c02e,
		0x070974d1, 0xd764e395, 0x74681675, 0xda4fa799, 0x1f1820d5, 0xbea0e6e7, 0x1fe3b6a6, 0x70ef0cca,
		0x311ad531, 0x3524b84d, 0x7dc7f8e3, 0xde581ae1, 0x432705f4, 0x07dbacba, 0x0abe69dc, 0x49ec8fa8,
		0x3f1658d7, 0x8ac1dbe5, 0x05c0cf41, 0x721cca9d, 0xa55fb1a2, 0x9b7023c4, 0x14e10484, 0xda907b80,
		0x0669dbfa, 0x400ff3a3, 0xd4cef3be, 0xd7cbc9e3, 0x3401405a, 0xf81468f2, 0x1ac58e38, 0x4b3dd6fe,
		0xfa050553, 0x8e451034, 0xfe6991dd, 0xf0eee0af, 0xdd7e48f3, 0x75dcad9f, 0xe5ac7a62, 0x67621b31,
		0x4d36cd20, 0x742198e0, 0x717909fb, 0x7fcd6736, 0x3cd65f77, 0xc6a6a2a2, 0x6acee31a, 0x6ca9cd4e,
		0x3b9dba86, 0xfd76f4b5, 0xbcf044f8, 0x296ee92e, 0x6b2f2523, 0x4427ab08, 0x99cc127a, 0x75f2dced,
		0x7e383cc5, 0xc51b1cf7, 0x65942dd1, 0xdd48c906, 0xac2d32be, 0x8132c9b5, 0x34d84a66, 0xdf153f35,
		0xb6ebeeb2, 0x964df604, 0x9c944235, 0x61d38a62, 0x6f7ba852, 0xf4fc61dc, 0xfe2d146c, 0x0aa4ea99,
		0x13fed9e8, 0x594448d0, 0xe3f36680, 0x198dd934, 0xfe63d716, 0x3a7e1830, 0xb10f8d9b, 0x8cf5f012,
		0xdb58780a, 0x8cb8633e, 0x8ef3aa3a, 0x2e1a8a37, 0xeff9315c, 0x7ee36de3, 0x133ebd9b, 0xb9c044c6,
		0x90da3abc, 0x74b0ada4, 0x892757f8, 0x373fe647, 0x5a7942e4, 0xee8d43df, 0xe8490ab4, 0x1a88c33c,
		0x766b0188, 0xa3fdc38a, 0x564e7a16, 0xbacb7fa7, 0xec1c5e02, 0x76c9b9b0, 0x39b1821e, 0xc557c93e,
		0x4c382419, 0xb8542f5d, 0x8e575d6f, 0x520aa130, 0x5e71186d, 0x59c30613, 0x623edc1f, 0xebb5dadc,
		0xf995911b, 0xdad591a7, 0x6bce5333, 0x017000f5, 0xe8eed87f, 0xcef10ac0, 0xd3b6eb63, 0xa5ccef78,
		0xa4bc5daa, 0xd2f2ab96, 0x9aeaff61, 0xa2ed6aa8, 0x61ed3ebd, 0x9282c139, 0xb1233616, 0xe524a0b0,
		0xaaa79b05, 0x339b120d, 0xda209283, 0xfcecb025, 0x2338d024, 0x74f295fc, 0x19e57380, 0x447d5097,
		0xdb449345, 0x691dada2, 0xe7ee1444, 0xff877f2c, 0xf1329e38, 0xda29bc4d, 0xfe262742, 0xa92bd2c1,
		0x0e7a42f6, 0xd17ce8cb, 0x56ec5b0f, 0x3161b769, 0x25f96db4, 0x6d793440, 0x0ba753fa, 0xce82a4fa,
		0x614945c3, 0x8f2c450d, 0xf7604928, 0x1ec97df3, 0xc189d00f, 0xd3f85226, 0x14358f4d, 0x0b5f9dba,
		0x004aa907, 0x2f2622f7, 0x1ffb673e, 0xc6119ca1, 0x665d4f69, 0x90153458, 0x4654e56c, 0xd6635faf,
		0xdf950c8a, 0xafe40dbd, 0x4c4040bf, 0x7151f6a3, 0xf826ed29, 0xd5222885, 0xfacfbebf, 0x517fc528,
		0x076306b8, 0x298fbdec, 0x717e55fa, 0x6632401a, 0x9dded4e8, 0x93fc5ed4, 0x3bd53d7a, 0x802e75cd,
		0x87744f0a, 0xea8fcc1b, 0x7cdba99a, 0xefe55316, 0x6ec178ab, 0x5a8972a4, 0x50702c98, 0x1fdfa1fb,
		0x44d9b76b, 0x56828007, 0x83c0bffd, 0x5bd0490e, 0x0e6a681e, 0x2f0bc29a, 0xe1a0438e, 0xb2f60c99,
		0x5e1c7ae0, 0x45a0c82c, 0x88e90b3c, 0xc696b9ac, 0x2a83ae74, 0x65fa13bb, 0xa61f4feb, 0xe18a8ab0,
		0xb9b8e981, 0x4e1555d5, 0x9badf245, 0x7e35c23e, 0x722e925f, 0x23685bb6, 0x0e45c66e, 0xd4873be9,
		0xe3c041f4, 0xbe4405a8, 0x138a0fe4, 0xf437c41a, 0xef55405a, 0x4b1d799d, 0x9c3a794a, 0xcc378576,
		0xb60f3d82, 0x7e93a660, 0xc4c25cbd, 0x907fc772, 0x10961b4d, 0x68680513, 0xff7bc035, 0x2a438546,
	},
	{ 0x01, 0x04, 0x05, 0x06, 0x02, 0x01, 0x05, 0x02, 0x00, 0x03, 0x07, 0x07, 0x05, 0x04, 0x06, 0x03 },
};

/************************************************************************/

DScm::DScm() :
	m_Valid(FALSE),
	m_Edit(FALSE),
	m_Version(L_SCM_VER_UNKNOWN),
	m_DdNum(0),
	m_Tile(NULL),
	m_Doodad(NULL),
	m_Archive(NULL)
{
	m_IsoMap.era = L_ERA_ERROR;
	m_IsoMap.def = 0;
	m_IsoMap.isom = NULL;
	m_IsoMap.tile = NULL;
	m_IsoMap.dirty = NULL;
	DVarClr(m_IsoMap.size);
}

DScm::~DScm()
{
#if 0
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

	DAssert(!m_IsoMap.isom && !m_Minimap.IsValid() && !m_Tile);

	if (!DBetween(ts.GetEra(), 0, L_ERA_NUM) || !CheckMapSize(size))
		return FALSE;

	m_IsoMap.era = ts.GetEra();
	m_IsoMap.def = def;
	m_IsoMap.size = size;

	if (!::CreateIsoMap(&m_IsoMap, TRUE)) {
		Clear();
		return FALSE;
	}

	// TODO: 随机TILE
#if 1
	m_Tile = new LTILEIDX[size.cx * size.cy];
	DMemCpy(m_Tile, m_IsoMap.tile, size.cx * size.cy * sizeof(LTILEIDX));
#endif

	if (!MakeMinimap(ts)) {
		Clear();
		return FALSE;
	}

	m_Valid = TRUE;
	m_Edit = TRUE;
	m_Version = L_SCM_VER_HYBRID;

	return TRUE;
}

BOOL DScm::Load(STRCPTR name, BOOL for_edit)
{
	if (m_Valid)
		return FALSE;

	if (!name)
		return FALSE;

	m_Archive = ::g_Archive.UseArchive(name, 0U);
	if (!m_Archive)
		return FALSE;

	m_Edit = for_edit;
	if (LoadMap()) {
		m_Valid = TRUE;
		return TRUE;
	}

	Clear();
	return FALSE;
}

BOOL DScm::Save(STRCPTR name) CONST
{
	if (!m_Valid || !m_Edit)
		return FALSE;

	if (!name)
		return FALSE;

	// TODO:
	return TRUE;
}

VOID DScm::Clear(VOID)
{
	m_Chk.Clear();
	m_Minimap.Destroy();

	if (m_Archive) {
		::g_Archive.CloseArchive(m_Archive);
		m_Archive = NULL;
	}

	::DestroyIsoMap(&m_IsoMap);
	m_IsoMap.era = L_ERA_ERROR;
	m_IsoMap.def = 0;
	m_IsoMap.isom = NULL;
	m_IsoMap.tile = NULL;
	m_IsoMap.dirty = NULL;
	DVarClr(m_IsoMap.size);

	m_DdNum = 0U;
	delete [] m_Doodad;
	m_Doodad = NULL;

	delete [] m_Tile;
	m_Tile = NULL;

	m_Version = L_SCM_VER_UNKNOWN;

	m_Edit = FALSE;
	m_Valid = FALSE;
}

BOOL DScm::GetEditable(VOID) CONST
{
	return m_Edit;
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
	if (!m_Valid || !m_Edit)
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
	if (!m_Valid || !m_Edit)
		return FALSE;

	return ::BrushIsoMap(&m_IsoMap, brush, &tile_pos);
}

BOOL DScm::UpdateTile(VOID)
{
	if (!m_Valid || !m_Edit)
		return FALSE;

	if (!::UpdateIsoMap(&m_IsoMap))
		return FALSE;

#if 1
	DMemCpy(m_Tile, m_IsoMap.tile, m_IsoMap.size.cx * m_IsoMap.size.cy * sizeof(LTILEIDX));
#endif
	return TRUE;
}

/************************************************************************/

BOOL DScm::LoadMap(VOID)
{
	if (!m_Chk.Load(CHK_FILE_PATH))
		return FALSE;

	if (!ReadVersion())
		return FALSE;
	if (!ReadEra())
		return FALSE;
	if (!ReadMapSize())
		return FALSE;
	if (!ReadTile())
		return FALSE;

	if (m_Edit) {
		if (!ReadIsoMap())
			return FALSE;
	} else {
		if (!ReadThingy())
			return FALSE;
	}

	if (!Verify())
		return FALSE;

	return TRUE;
}

BOOL DScm::Verify(VOID)
{
	UINT size = m_Chk.GetSectionSize(FOURCC_VCOD, DChk::ST_LASTONE);
	if (size != sizeof(VCODE))
		return FALSE;

	VCODE vcode;
	if (!m_Chk.GetSectionData(FOURCC_VCOD, DChk::ST_LASTONE, &vcode, sizeof(vcode)))
		return FALSE;

	if (DMemCmp(&vcode, &VERIFY_CODE, sizeof(vcode)))
		return FALSE;

	return TRUE;
}

BOOL DScm::ReadVersion(VOID)
{
	WORD version;

	UINT size = m_Chk.GetSectionSize(FOURCC_VER, DChk::ST_LASTONE);
	if (size != sizeof(version))
		return FALSE;

	if (!m_Chk.GetSectionData(FOURCC_VER, DChk::ST_LASTONE, &version, sizeof(version)))
		return FALSE;

	switch (version) {
	case 0x39:
		m_Version = L_SCM_VER_BETA;
		break;
	case 0x3b:
		m_Version = L_SCM_VER_STARCRAFT;
		break;
	case 0x3f:
		m_Version = L_SCM_VER_HYBRID;
		break;
	case 0xcd:
		m_Version = L_SCM_VER_BROODWAR;
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL DScm::ReadEra(VOID)
{
	ERA_DATA era;

	UINT size = m_Chk.GetSectionSize(FOURCC_ERA, DChk::ST_LASTONE);
	if (size != sizeof(era))
		return FALSE;

	if (!m_Chk.GetSectionData(FOURCC_ERA, DChk::ST_LASTONE, &era, sizeof(era)))
		return FALSE;

	if (era.era >= L_ERA_NUM)
		return FALSE;

	m_IsoMap.era = era.era;
	return TRUE;
}

BOOL DScm::ReadMapSize(VOID)
{
	DIM_DATA dim;
	SIZE map_size;

	UINT size = m_Chk.GetSectionSize(FOURCC_DIM, DChk::ST_LASTONE);
	if (size != sizeof(dim))
		return FALSE;

	if (!m_Chk.GetSectionData(FOURCC_DIM, DChk::ST_LASTONE, &dim, sizeof(dim)))
		return FALSE;

	map_size.cx = dim.width;
	map_size.cy = dim.height;
	if (!CheckMapSize(map_size))
		return FALSE;

	m_IsoMap.size = map_size;
	return TRUE;
}

BOOL DScm::ReadTile(VOID)
{
	DAssert(!m_Tile);

	UINT size = m_Chk.GetSectionSize(FOURCC_MTXM, DChk::ST_OVERRIDE);
	if (size > DIM_HUGE * DIM_HUGE * sizeof(LTILEIDX))
		return FALSE;

	UINT map_size = m_IsoMap.size.cx * m_IsoMap.size.cy;
	DAssert(map_size > 0);

	m_Tile = new LTILEIDX[map_size];
	if (!m_Tile)
		return FALSE;

	map_size *= sizeof(LTILEIDX);
	DMemClr(m_Tile, map_size);

	UINT rd_size = DMin(map_size, size);
	if (rd_size && !m_Chk.GetSectionData(FOURCC_MTXM, DChk::ST_OVERRIDE, m_Tile, rd_size))
		return FALSE;

	return TRUE;
}

BOOL DScm::ReadIsoMap(VOID)
{
	DAssert(!m_IsoMap.isom && !m_IsoMap.tile && !m_Doodad);

	if (!::CreateIsoMap(&m_IsoMap, DChk::ST_OVERRIDE))
		return FALSE;

	UINT tile_size = m_Chk.GetSectionSize(FOURCC_TILE, DChk::ST_OVERRIDE);
	if (tile_size) {
		UINT full_size = m_IsoMap.size.cx * m_IsoMap.size.cy * sizeof(LTILEIDX);
		UINT copy_size = DMin(tile_size, full_size);
		if (!m_Chk.GetSectionData(FOURCC_TILE, DChk::ST_OVERRIDE, m_IsoMap.tile, copy_size))
			return FALSE;
	}

	UINT isom_size = m_Chk.GetSectionSize(FOURCC_ISOM, DChk::ST_OVERRIDE);
	if (isom_size) {
		UINT full_size = CALC_ISOM_ROW(m_IsoMap.size.cx) * CALC_ISOM_LINE(m_IsoMap.size.cy) * sizeof(LISOMTILE);
		UINT copy_size = DMin(isom_size, full_size);
		if (!m_Chk.GetSectionData(FOURCC_ISOM, DChk::ST_OVERRIDE, m_IsoMap.isom, copy_size))
			return FALSE;
	}

	UINT dd2_size = m_Chk.GetSectionSize(FOURCC_DD2, DChk::ST_DUPLICATE);
	UINT dd_num = dd2_size / sizeof(DD2_DATA);
	if (dd_num) {
		dd2_size = dd_num * sizeof(DD2_DATA);
		DArray<DD2_DATA> dd2(dd2_size);
		if (!dd2.IsNull()) {
			if (!m_Chk.GetSectionData(FOURCC_DD2, DChk::ST_DUPLICATE, dd2, dd2_size))
				return FALSE;
			LDDPTR doodad = new LDOODAD[dd_num];
			if (doodad) {
				for (UINT i = 0U; i < dd_num; i++) {
					doodad[i].dd_no = dd2[i].dd_no;
					doodad[i].x = dd2[i].x;
					doodad[i].y = dd2[i].y;
					doodad[i].owner = dd2[i].owner;
				}
				m_DdNum = dd_num;
				m_Doodad = doodad;
			}
		}
	}

	// TODO: recover TILE and ISOM
	return TRUE;
}

BOOL DScm::ReadThingy(VOID)
{
	UINT size = m_Chk.GetSectionSize(FOURCC_THG2, DChk::ST_DUPLICATE);
	if (size % sizeof(THG2_DATA))
		return FALSE;

	UINT thg_num = size / sizeof(THG2_DATA);
	if (!thg_num)
		return TRUE;

	THG2_DATA *thg2 = new THG2_DATA[thg_num];
	if (thg2 && m_Chk.GetSectionData(FOURCC_THG2, DChk::ST_DUPLICATE, thg2, thg_num * sizeof(THG2_DATA))) {

	}

	// TODO:
	delete [] thg2;
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
	} else if (max_dim > DIM_MEDIUM) {
		size.cx /= 2;
		size.cy /= 2;
		MakeLargeMinimap(ts, size);
	} else {
		MakeMediumMinimap(ts, size);
	}

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

DWORD DScm::CalcVerifyHash(VCODE *vcode, UINT vcode_size, VCPTR vdata, UINT vdata_size)
{
	DAssert(vcode && vcode_size && vdata && vdata_size);

	if (vcode_size <= sizeof(vcode->code))
		return 0UL;

	UINT op_num = vcode_size - sizeof(vcode->code);
	DWORD hash = 0UL;
	BUFCPTR ptr = static_cast<BUFCPTR>(vdata);

	vdata_size >>= 2;

	for (UINT i = 0; i < vdata_size; i++, ptr += 4) {
		for (UINT j = 0; j < op_num; j++) {
			switch (vcode->op[j]) {
			case 0x00:
				hash ^= *reinterpret_cast<CONST DWORD *>(ptr);
				break;
			case 0x01:
				hash += *reinterpret_cast<CONST DWORD *>(ptr);
				break;
			case 0x02:
				hash -= *reinterpret_cast<CONST DWORD *>(ptr);
				break;
			case 0x03:
			case 0x04:
			case 0x05:
				hash ^= vcode->code[ptr[0]];
				hash ^= vcode->code[ptr[1]];
				hash ^= vcode->code[ptr[2]];
				hash ^= vcode->code[ptr[3]];
				break;
			case 0x06:
				hash = SAFE_ROL(hash, ptr[0]);
				hash = SAFE_ROL(hash, ptr[1]);
				hash = SAFE_ROL(hash, ptr[2]);
				hash = SAFE_ROL(hash, ptr[3]);
				break;
			case 0x07:
				hash = SAFE_ROR(hash, ptr[0]);
				hash = SAFE_ROR(hash, ptr[1]);
				hash = SAFE_ROR(hash, ptr[2]);
				hash = SAFE_ROR(hash, ptr[3]);
				break;
			default:
				break;
			}
		}
	}

	return hash;
}

/************************************************************************/
