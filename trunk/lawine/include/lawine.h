/************************************************************************/
/* File Name   : lawine.h                                               */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 21st, 2008                                         */
/* Module      : Lawine library                                         */
/* Descript    : Lawine API definition                                  */
/************************************************************************/

#ifndef __SD_LAWINE_LAWINE_H__
#define __SD_LAWINE_LAWINE_H__

/************************************************************************/

#include <dsound.h>
#include <common.h>
#include <image.h>
#include <lawinedef.h>

/************************************************************************/

#ifdef WIN32
#ifdef LAWINE_EXPORTS
#define LAWINE_API	__declspec(dllexport) WINAPI
#else
#define LAWINE_API	WINAPI
#endif
#else
#define LAWINE_API	
#endif

/************************************************************************/

typedef HANDLE		LHFILE;

#if defined(LAWINE_EXPORTS) && defined(__cplusplus)
#include "data/mpq.hpp"
#include "data/pcx.hpp"
#include "data/spk.hpp"
#include "data/grp.hpp"
#include "data/fnt.hpp"
#include "data/smk.hpp"
#include "data/scm.hpp"
#include "data/tileset.hpp"
typedef DMpq		*LHMPQ;
typedef DPcx		*LHPCX;
typedef DSpk		*LHSPK;
typedef DGrp		*LHGRP;
typedef DFnt		*LHFNT;
typedef DSmk		*LHSMK;
typedef DScm		*LHSCM;
typedef DTileset	*LHTILESET;
#else
typedef HANDLE		LHMPQ;
typedef HANDLE		LHPCX;
typedef HANDLE		LHSPK;
typedef HANDLE		LHGRP;
typedef HANDLE		LHFNT;
typedef HANDLE		LHSMK;
typedef HANDLE		LHSCM;
typedef HANDLE		LHTILESET;
#endif

/************************************************************************/

CAPI extern BOOL LAWINE_API InitLawine(VOID);
CAPI extern VOID LAWINE_API ExitLawine(VOID);

CAPI extern BOOL LAWINE_API LInitMpq(VOID);
CAPI extern VOID LAWINE_API LExitMpq(VOID);

CAPI extern BOOL LAWINE_API LInitSmack(LPDIRECTSOUND ds);
CAPI extern VOID LAWINE_API LExitSmack(VOID);

CAPI extern BOOL LAWINE_API LInitFont(VOID);
CAPI extern VOID LAWINE_API LExitFont(VOID);

CAPI extern BOOL LAWINE_API LInitMap(VOID);
CAPI extern VOID LAWINE_API LExitMap(VOID);

CAPI extern BOOL LAWINE_API LCycleColor(PALPTR pal);
CAPI extern BOOL LAWINE_API LGetUserColor(PALPTR pal, INT user);

CAPI extern LHMPQ LAWINE_API LMpqCreate(STRCPTR name, UINT *hash_num);
CAPI extern LHMPQ LAWINE_API LMpqOpen(STRCPTR name);
CAPI extern BOOL LAWINE_API LMpqClose(LHMPQ mpq);
CAPI extern BOOL LAWINE_API LMpqFileExist(LHMPQ mpq, STRCPTR file_name);
CAPI extern BOOL LAWINE_API LMpqAddFile(LHMPQ mpq, STRCPTR file_name, STRCPTR physic_path, BOOL compress, BOOL encrypt);
CAPI extern BOOL LAWINE_API LMpqNewFile(LHMPQ mpq, STRCPTR file_name, BUFCPTR file_data, UINT size, BOOL compress, BOOL encrypt);
CAPI extern BOOL LAWINE_API LMpqDelFile(LHMPQ mpq, STRCPTR file_name);
CAPI extern LHFILE LAWINE_API LMpqOpenFile(LHMPQ mpq, STRCPTR file_name);
CAPI extern BOOL LAWINE_API LMpqCloseFile(LHMPQ mpq, LHFILE file);
CAPI extern HANDLE LAWINE_API LMpqOpenHandle(LHMPQ mpq, STRCPTR file_name);
CAPI extern UINT LAWINE_API LMpqGetFileSize(LHFILE file);
CAPI extern UINT LAWINE_API LMpqReadFile(LHFILE file, VPTR data, UINT size);
CAPI extern UINT LAWINE_API LMpqSeekFile(LHFILE file, INT offset, SEEK_MODE mode);

CAPI extern LHMPQ LAWINE_API LArcUseArchive(STRCPTR arc_name, UINT priority);
CAPI extern BOOL LAWINE_API LArcClose(LHMPQ arc);
CAPI extern BOOL LAWINE_API LArcFileExist(STRCPTR file_name);
CAPI extern LHFILE LAWINE_API LArcOpenFile(STRCPTR file_name);
CAPI extern BOOL LAWINE_API LArcCloseFile(LHFILE file);
CAPI extern HANDLE LAWINE_API LArcOpenHandle(STRCPTR file_name);

CAPI extern LHPCX LAWINE_API LPcxCreate(IMGCPTR img, PALCPTR pal);
CAPI extern LHPCX LAWINE_API LPcxOpen(STRCPTR name);
CAPI extern BOOL LAWINE_API LPcxSave(LHPCX pcx, STRCPTR name);
CAPI extern BOOL LAWINE_API LPcxClose(LHPCX pcx);
CAPI extern BOOL LAWINE_API LPcxDecode(LHPCX pcx);
CAPI extern IMGCPTR LAWINE_API LPcxGetImage(LHPCX pcx);
CAPI extern PALCPTR LAWINE_API LPcxGetPalette(LHPCX pcx);

CAPI extern LHSPK LAWINE_API LSpkOpen(STRCPTR name);
CAPI extern BOOL LAWINE_API LSpkClose(LHSPK spk);
CAPI extern INT LAWINE_API LSpkLayerNum(LHSPK spk);
CAPI extern BOOL LAWINE_API LSpkGetImage(LHSPK spk, IMGPTR img, CONST POINT *view_pos);

CAPI extern LHGRP LAWINE_API LGrpOpen(STRCPTR name);
CAPI extern BOOL LAWINE_API LGrpClose(LHGRP grp);
CAPI extern INT LAWINE_API LGrpFrameNum(LHGRP grp);
CAPI extern BOOL LAWINE_API LGrpDecode(LHGRP grp, INT frame_no);
CAPI extern IMGCPTR LAWINE_API LGrpGetImage(LHGRP grp);

CAPI extern LHFNT LAWINE_API LFntOpen(STRCPTR name, BOOL crypt);
CAPI extern BOOL LAWINE_API LFntClose(LHFNT fnt);
CAPI extern BOOL LAWINE_API LFntMaxSize(LHFNT fnt, SIZE *size);
CAPI extern BOOL LAWINE_API LFntSetTemp(LHFNT fnt, IMGCPTR temp);
CAPI extern BOOL LAWINE_API LFntCharSize(LHFNT fnt, BYTE ch, SIZE *size);
CAPI extern BOOL LAWINE_API LFntGetChar(LHFNT fnt, IMGPTR img, BYTE ch, INT style);

CAPI extern LHSMK LAWINE_API LSmkOpen(STRCPTR name);
CAPI extern BOOL LAWINE_API LSmkClose(LHSMK smk);
CAPI extern BOOL LAWINE_API LSmkSize(LHSMK smk, SIZE *size);
CAPI extern INT LAWINE_API LSmkFrameNum(LHSMK smk);
CAPI extern INT LAWINE_API LSmkCurFrame(LHSMK smk);
CAPI extern INT LAWINE_API LSmkRestLoop(LHSMK smk);
CAPI extern BOOL LAWINE_API LSmkPlay(LHSMK smk);
CAPI extern BOOL LAWINE_API LSmkStart(LHSMK smk, INT loop);
CAPI extern BOOL LAWINE_API LSmkStop(LHSMK smk);
CAPI extern BOOL LAWINE_API LSmkPause(LHSMK smk);
CAPI extern BOOL LAWINE_API LSmkResume(LHSMK smk);
CAPI extern IMGCPTR LAWINE_API LSmkGetImage(LHSMK smk);

CAPI extern LHSCM LAWINE_API LScmCreate(LHTILESET ts, INT def, CONST SIZE *size);
CAPI extern LHSCM LAWINE_API LScmOpen(STRCPTR name, BOOL for_edit);
CAPI extern BOOL LAWINE_API LScmSave(LHSCM scm, STRCPTR name);
CAPI extern BOOL LAWINE_API LScmClose(LHSCM scm);
CAPI extern BOOL LAWINE_API LScmEditable(LHSCM scm);
CAPI extern INT LAWINE_API LScmGetVersion(LHSCM scm);
CAPI extern INT LAWINE_API LScmGetEra(LHSCM scm);
CAPI extern BOOL LAWINE_API LScmGetSize(LHSCM scm, SIZE *size);
CAPI extern IMGCPTR LAWINE_API LScmGetMinimap(LHSCM scm);
CAPI extern BOOL LAWINE_API LScmGenMinimap(LHSCM scm, LHTILESET ts);
CAPI extern LTILECPTR LAWINE_API LScmTileData(LHSCM scm);
CAPI extern LISOMCPTR LAWINE_API LScmIsoMapData(LHSCM scm);
CAPI extern BOOL LAWINE_API LScmIsoBrush(LHSCM scm, INT brush, CONST POINT *tile_pos);
CAPI extern BOOL LAWINE_API LScmUpdate(LHSCM scm);

CAPI extern LHTILESET LAWINE_API LTsOpen(INT era, BOOL no_cycling);
CAPI extern BOOL LAWINE_API LTsClose(LHTILESET ts);
CAPI extern INT LAWINE_API LTsGetEra(LHTILESET ts);
CAPI extern BOOL LAWINE_API LTsGetTile(LHTILESET ts, LTILEIDX index, IMGPTR img);
CAPI extern BOOL LAWINE_API LTsInitIsoMap(LHTILESET ts);
CAPI extern BOOL LAWINE_API LTsExitIsoMap(LHTILESET ts);
CAPI extern PALCPTR LAWINE_API LTsGetPalette(LHTILESET ts);

/************************************************************************/

#endif	/* __SD_LAWINE_LAWINE_H__ */
