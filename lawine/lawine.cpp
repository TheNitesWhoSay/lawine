/************************************************************************/
/* File Name   : lawine.cpp                                             */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 5th, 2009                                          */
/* Module      : Lawine library                                         */
/* Descript    : Lawine API implementation                              */
/************************************************************************/

#include <lawine.h>
#include "global.hpp"
#include "misc/color.h"
#include "misc/smack.h"
#include "misc/isomap.h"
#include "data/mpq.hpp"
#include "data/pcx.hpp"
#include "data/grp.hpp"
#include "data/fnt.hpp"
#include "data/tileset.hpp"

/************************************************************************/

CAPI BOOL LAWINE_API InitLawine(VOID)
{
	return TRUE;
}

CAPI VOID LAWINE_API ExitLawine(VOID)
{

}

CAPI BOOL LAWINE_API LInitMpq(VOID)
{
	return DMpq::Initialize();
}

CAPI VOID LAWINE_API LExitMpq(VOID)
{
	DMpq::Exit();
}

CAPI BOOL LAWINE_API LInitSmack(LPDIRECTSOUND ds)
{
	if (!::InitSmacker())
		return FALSE;

	/* smackw32.dll使用MMX指令加速 */
	::SmackUseMMX(SMACK_MMX_ON);

	/* smackw32.dll使用DS */
	::SmackSoundUseDirectSound(ds);

	return TRUE;
}

CAPI VOID LAWINE_API LExitSmack(VOID)
{
	::ExitSmacker();
}

CAPI BOOL LAWINE_API LInitFont(VOID)
{
	return DFnt::Initialize();
}

CAPI VOID LAWINE_API LExitFont(VOID)
{
	DFnt::Exit();
}

CAPI BOOL LAWINE_API LInitMap(VOID)
{
	return ::InitIsoMap();
}

CAPI VOID LAWINE_API LExitMap(VOID)
{
	::ExitIsoMap();
}

/************************************************************************/

CAPI BOOL LAWINE_API LCycleColor(PALPTR pal)
{
	return ::CycleColor(pal);
}

CAPI BOOL LAWINE_API LGetUserColor(PALPTR pal, INT user)
{
	return ::GetUserColor(pal, user);
}

/************************************************************************/

CAPI LHMPQ LAWINE_API LMpqUseArchive(STRCPTR mpq_name, UINT priority)
{
	return ::g_Archive.UseArchive(mpq_name, priority);
}

CAPI BOOL LAWINE_API LMpqCloseArchive(LHMPQ mpq)
{
	return ::g_Archive.CloseArchive(mpq);
}

CAPI BOOL LAWINE_API LMpqFileExist(STRCPTR file_name)
{
	return ::g_Archive.FileExist(file_name);
}

CAPI LHFILE LAWINE_API LMpqOpenFile(STRCPTR file_name)
{
	return ::g_Archive.OpenFile(file_name);
}

CAPI BOOL LAWINE_API LMpqCloseFile(LHFILE file)
{
	return ::g_Archive.CloseFile(file);
}

CAPI UINT LAWINE_API LMpqGetFileSize(LHFILE file)
{
	return ::g_Archive.GetFileSize(file);
}

CAPI UINT LAWINE_API LMpqReadFile(LHFILE file, VPTR data, UINT size)
{
	return ::g_Archive.ReadFile(file, data, size);
}

CAPI UINT LAWINE_API LMpqSeekFile(LHFILE file, INT offset, SEEK_MODE mode)
{
	return ::g_Archive.SeekFile(file, offset, mode);
}

CAPI HANDLE LAWINE_API LMpqOpenHandle(STRCPTR file_name)
{
	return ::g_Archive.OpenHandle(file_name);
}

/************************************************************************/

CAPI LHPCX LAWINE_API LPcxCreate(IMGCPTR img, PALCPTR pal)
{
	if (!img)
		return NULL;

	DPcx *pcx = new DPcx;
	if (!pcx)
		return NULL;

	if (pcx->Create(DImage(*const_cast<IMGPTR>(img)), DPalette(const_cast<PALPTR>(pal))))
		return pcx;

	delete pcx;
	return NULL;
}

CAPI LHPCX LAWINE_API LPcxOpen(STRCPTR name)
{
	DPcx *pcx = new DPcx;
	if (!pcx)
		return NULL;

	if (pcx->Load(name))
		return pcx;

	delete pcx;
	return NULL;
}

CAPI BOOL LAWINE_API LPcxSave(LHPCX pcx, STRCPTR name)
{
	if (!pcx)
		return FALSE;

	return pcx->Save(name);
}

CAPI BOOL LAWINE_API LPcxClose(LHPCX pcx)
{
	if (!pcx)
		return FALSE;

	pcx->Clear();
	delete pcx;
	return TRUE;
}

CAPI BOOL LAWINE_API LPcxDecode(LHPCX pcx)
{
	if (!pcx)
		return NULL;

	return pcx->Decode();
}

CAPI IMGCPTR LAWINE_API LPcxGetImage(LHPCX pcx)
{
	if (!pcx)
		return NULL;

	CONST DImage *img = pcx->GetImage();
	if (!img)
		return NULL;

	return img->GetImage();
}

CAPI PALCPTR LAWINE_API LPcxGetPalette(LHPCX pcx)
{
	if (!pcx)
		return NULL;

	CONST DPalette *pal = pcx->GetPalette();
	if (!pal)
		return NULL;

	return pal->GetPalette();
}

/************************************************************************/

CAPI LHSPK LAWINE_API LSpkOpen(STRCPTR name)
{
	DSpk *spk = new DSpk;
	if (!spk)
		return NULL;

	if (spk->Load(name))
		return spk;

	delete spk;
	return NULL;
}

CAPI BOOL LAWINE_API LSpkClose(LHSPK spk)
{
	if (!spk)
		return FALSE;

	spk->Clear();
	delete spk;
	return TRUE;
}

CAPI INT LAWINE_API LSpkLayerNum(LHSPK spk)
{
	if (!spk)
		return 0;

	return spk->GetLayerNum();
}

CAPI BOOL LAWINE_API LSpkGetImage(LHSPK spk, IMGPTR img, CONST POINT *view_pos)
{
	if (!spk || !img || !view_pos)
		return NULL;

	return spk->GetImage(DImage(*img), *view_pos);
}

/************************************************************************/

CAPI LHGRP LAWINE_API LGrpOpen(STRCPTR name)
{
	DGrp *grp = new DGrp;
	if (!grp)
		return NULL;

	if (grp->Load(name))
		return grp;

	delete grp;
	return NULL;
}

CAPI BOOL LAWINE_API LGrpClose(LHGRP grp)
{
	if (!grp)
		return FALSE;

	grp->Clear();
	delete grp;
	return TRUE;
}

CAPI INT LAWINE_API LGrpFrameNum(LHGRP grp)
{
	if (!grp)
		return 0;

	return grp->GetFrameNum();
}

CAPI BOOL LAWINE_API LGrpDecode(LHGRP grp, INT frame_no)
{
	if (!grp)
		return FALSE;

	return grp->Decode(frame_no);
}

CAPI IMGCPTR LAWINE_API LGrpGetImage(LHGRP grp)
{
	if (!grp)
		return NULL;

	CONST DImage *img = grp->GetImage();
	if (!img)
		return NULL;

	return img->GetImage();
}

/************************************************************************/

CAPI LHFNT LAWINE_API LFntOpen(STRCPTR name, BOOL crypt)
{
	DFnt *fnt = new DFnt;
	if (!fnt)
		return NULL;

	if (fnt->Load(name))
		return fnt;

	delete fnt;
	return NULL;
}

CAPI BOOL LAWINE_API LFntClose(LHFNT fnt)
{
	if (!fnt)
		return FALSE;

	fnt->Clear();
	delete fnt;
	return TRUE;
}

CAPI BOOL LAWINE_API LFntMaxSize(LHFNT fnt, SIZE *size)
{
	if (!fnt || !size)
		return FALSE;

	return fnt->GetMaxSize(*size);
}

CAPI BOOL LAWINE_API LFntSetTemp(LHFNT fnt, IMGCPTR temp)
{
	if (!fnt)
		return FALSE;

	return fnt->SetTemplate(temp);
}

CAPI BOOL LAWINE_API LFntCharSize(LHFNT fnt, BYTE ch, SIZE *size)
{
	if (!fnt || !size)
		return FALSE;

	return fnt->GetCharSize(ch, *size);
}

CAPI BOOL LAWINE_API LFntGetChar(LHFNT fnt, IMGPTR img, BYTE ch, INT style)
{
	if (!fnt || !img)
		return FALSE;

	return fnt->GetChar(DImage(*img), ch, style);
}

/************************************************************************/

CAPI LHSMK LAWINE_API LSmkOpen(STRCPTR name)
{
	DSmk *smk = new DSmk;
	if (!smk)
		return NULL;

	if (smk->Load(name))
		return smk;

	delete smk;
	return NULL;
}

CAPI BOOL LAWINE_API LSmkClose(LHSMK smk)
{
	if (!smk)
		return FALSE;

	smk->Clear();
	delete smk;
	return TRUE;
}

CAPI BOOL LAWINE_API LSmkSize(LHSMK smk, SIZE *size)
{
	if (!smk || !size)
		return FALSE;

	return smk->GetSize(*size);
}

CAPI INT LAWINE_API LSmkFrameNum(LHSMK smk)
{
	if (!smk)
		return 0;

	return smk->GetFrameNum();
}

CAPI INT LAWINE_API LSmkCurFrame(LHSMK smk)
{
	if (!smk)
		return -1;

	return smk->GetCurFrame();
}

CAPI INT LAWINE_API LSmkRestLoop(LHSMK smk)
{
	if (!smk)
		return 0;

	return smk->GetRestLoop();
}

CAPI BOOL LAWINE_API LSmkPlay(LHSMK smk)
{
	if (!smk)
		return FALSE;

	return smk->Play();
}

CAPI BOOL LAWINE_API LSmkStart(LHSMK smk, INT loop)
{
	if (!smk)
		return FALSE;

	return smk->Start(loop);
}

CAPI BOOL LAWINE_API LSmkStop(LHSMK smk)
{
	if (!smk)
		return FALSE;

	return smk->Stop();
}

CAPI BOOL LAWINE_API LSmkPause(LHSMK smk)
{
	if (!smk)
		return FALSE;

	return smk->Pause();
}

CAPI BOOL LAWINE_API LSmkResume(LHSMK smk)
{
	if (!smk)
		return FALSE;

	return smk->Resume();
}

CAPI IMGCPTR LAWINE_API LSmkGetImage(LHSMK smk)
{
	if (!smk)
		return FALSE;

	CONST DImage *img = smk->GetImage();
	if (!img)
		return NULL;

	return img->GetImage();
}

/************************************************************************/

CAPI LHSCM LAWINE_API LScmCreate(LHTILESET ts, INT def, CONST SIZE *size)
{
	if (!ts || !size)
		return NULL;

	DScm *scm = new DScm;
	if (!scm)
		return NULL;

	if (scm->Create(*ts, def, *size))
		return scm;

	delete scm;
	return NULL;
}

CAPI LHSCM LAWINE_API LScmOpen(STRCPTR name, BOOL for_edit)
{
	DScm *scm = new DScm;
	if (!scm)
		return NULL;

	if (scm->Load(name, for_edit))
		return scm;

	delete scm;
	return NULL;
}

CAPI BOOL LAWINE_API LScmSave(LHSCM scm, STRCPTR name)
{
	if (!scm)
		return FALSE;

	return scm->Save(name);
}

CAPI BOOL LAWINE_API LScmClose(LHSCM scm)
{
	if (!scm)
		return FALSE;

	scm->Clear();
	delete scm;
	return TRUE;
}

CAPI BOOL LAWINE_API LScmEditable(LHSCM scm)
{
	if (!scm)
		return FALSE;

	return scm->GetEditable();
}

CAPI INT LAWINE_API LScmGetVersion(LHSCM scm)
{
	if (!scm)
		return L_SCM_VER_UNKNOWN;

	return scm->GetVersion();
}

CAPI INT LAWINE_API LScmGetEra(LHSCM scm)
{
	if (!scm)
		return L_ERA_ERROR;

	return scm->GetEra();
}

CAPI BOOL LAWINE_API LScmGetSize(LHSCM scm, SIZE *size)
{
	if (!scm || !size)
		return FALSE;

	return scm->GetSize(*size);
}

CAPI IMGCPTR LAWINE_API LScmGetMinimap(LHSCM scm)
{
	if (!scm)
		return NULL;

	return scm->GetMinimap();
}

CAPI BOOL LAWINE_API LScmGenMinimap(LHSCM scm, LHTILESET ts)
{
	if (!scm || !ts)
		return FALSE;

	return scm->GenMinimap(*ts);
}

CAPI LTILECPTR LAWINE_API LScmTileData(LHSCM scm)
{
	if (!scm)
		return NULL;

	return scm->GetTileData();
}

CAPI LISOMCPTR LAWINE_API LScmIsoMapData(LHSCM scm)
{
	if (!scm)
		return NULL;

	return scm->GetIsoMapData();
}

CAPI IMGCPTR LAWINE_API LScmMinimap(LHSCM scm)
{
	if (!scm)
		return NULL;

	return scm->GetMinimap();
}

CAPI BOOL LAWINE_API LScmIsoBrush(LHSCM scm, INT brush, CONST POINT *tile_pos)
{
	if (!scm || !tile_pos)
		return FALSE;

	return scm->IsoBrush(brush, *tile_pos);
}

CAPI BOOL LAWINE_API LScmUpdate(LHSCM scm)
{
	if (!scm)
		return FALSE;

	return scm->UpdateTile();
}

/************************************************************************/

CAPI LHTILESET LAWINE_API LTsOpen(INT era, BOOL no_cycling)
{
	DTileset *ts = new DTileset;
	if (!ts)
		return NULL;

	if (ts->Load(era, no_cycling))
		return ts;

	delete ts;
	return NULL;
}

CAPI BOOL LAWINE_API LTsClose(LHTILESET ts)
{
	if (!ts)
		return FALSE;

	ts->Clear();
	delete ts;
	return TRUE;
}

CAPI INT LAWINE_API LTsGetEra(LHTILESET ts)
{
	if (!ts)
		return FALSE;

	return ts->GetEra();
}

CAPI BOOL LAWINE_API LTsGetTile(LHTILESET ts, LTILEIDX index, IMGPTR img)
{
	if (!ts || !img)
		return FALSE;

	return ts->GetTile(index, DImage(*img));
}

CAPI BOOL LAWINE_API LTsInitIsoMap(LHTILESET ts)
{
	if (!ts)
		return FALSE;

	return ts->InitIsoMap();
}

CAPI BOOL LAWINE_API LTsExitIsoMap(LHTILESET ts)
{
	if (!ts)
		return FALSE;

	ts->ExitIsoMap();
	return TRUE;
}

CAPI PALCPTR LAWINE_API LTsGetPalette(LHTILESET ts)
{
	if (!ts)
		return NULL;

	CONST DPalette *pal = ts->GetPalette();
	if (!pal)
		return NULL;

	return pal->GetPalette();
}

/************************************************************************/
