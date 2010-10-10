/************************************************************************/
/* File Name   : smack.c                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 24th, 2008                                         */
/* Module      : Lawine library                                         */
/* Descript    : Smacker library definition                             */
/************************************************************************/

#include "smack.h"

/************************************************************************/

#define SMACK_DLL_FILE				"smackw32.dll"
#define WINAPI_SYMBOL(s, n)			"_"###s##"@"###n

/************************************************************************/

DWORD (WINAPI *SmackUseMMX)(DWORD flag);						// 0 = off, 1 = on, 2 = query current
SMACK *(WINAPI *SmackOpen)(LPCSTR name, UINT flags, VPTR extra_buf);
INT (WINAPI *SmackDoFrame)(SMACK *smk);
VOID (WINAPI *SmackNextFrame)(SMACK *smk);
INT (WINAPI *SmackWait)(SMACK *smk);
VOID (WINAPI *SmackClose)(SMACK *smk);
VOID (WINAPI *SmackVolumePan)(SMACK *smk, UINT track_flag, UINT volume, UINT pan);
INT (WINAPI *SmackSoundInTrack)(SMACK *smk, UINT track_flags);
INT (WINAPI *SmackSoundOnOff)(SMACK *smk, BOOL on);
VOID (WINAPI *SmackToBuffer)(SMACK *smk, INT left, INT top, UINT pitch, INT dest_height, VPTR buf, UINT flags);
BOOL (WINAPI *SmackToBufferRect)(SMACK *smk, DWORD smack_surface);
VOID (WINAPI *SmackGoto)(SMACK *smk, INT frame);
VOID (WINAPI *SmackColorRemap)(SMACK *smk, VPTR remap_pal, UINT num_colors, INT pal_type);
VOID (WINAPI *SmackColorTrans)(SMACK *smk, VCPTR trans);
VOID (WINAPI *SmackFrameRate)(DWORD force_rate);
INT (WINAPI *SmackGetTrackData)(SMACK *smk, VPTR dest, UINT track_flag);
BYTE (WINAPI *SmackSoundUseDirectSound)(LPDIRECTSOUND ds);		// NULL = Create

/************************************************************************/

static HMODULE s_Smacker;

/************************************************************************/

BOOL InitSmacker(VOID)
{
	if (s_Smacker)
		return TRUE;

	s_Smacker = LoadLibrary(SMACK_DLL_FILE);
	if (!s_Smacker)
		return FALSE;

#define LOAD_FUNCTION(name, param_num)	do { \
		name = (VPTR)GetProcAddress(s_Smacker, WINAPI_SYMBOL(name, param_num)); \
		if (!name) \
			return FALSE; \
	} while (0)

	LOAD_FUNCTION(SmackUseMMX, 4);
	LOAD_FUNCTION(SmackOpen, 12);
	LOAD_FUNCTION(SmackDoFrame, 4);
	LOAD_FUNCTION(SmackNextFrame, 4);
	LOAD_FUNCTION(SmackWait, 4);
	LOAD_FUNCTION(SmackClose, 4);
	LOAD_FUNCTION(SmackVolumePan, 16);
	LOAD_FUNCTION(SmackSoundInTrack, 8);
	LOAD_FUNCTION(SmackSoundOnOff, 8);
	LOAD_FUNCTION(SmackToBuffer, 28);
	LOAD_FUNCTION(SmackToBufferRect, 8);
	LOAD_FUNCTION(SmackGoto, 8);
	LOAD_FUNCTION(SmackColorRemap, 16);
	LOAD_FUNCTION(SmackColorTrans, 8);
	LOAD_FUNCTION(SmackFrameRate, 4);
	LOAD_FUNCTION(SmackGetTrackData, 12);
	LOAD_FUNCTION(SmackSoundUseDirectSound, 4);

#undef LOAD_FUNCTION

	return TRUE;
}

VOID ExitSmacker(VOID)
{
	SmackUseMMX = NULL;
	SmackOpen = NULL;
	SmackDoFrame = NULL;
	SmackNextFrame = NULL;
	SmackWait = NULL;
	SmackClose = NULL;
	SmackVolumePan = NULL;
	SmackSoundInTrack = NULL;
	SmackSoundOnOff = NULL;
	SmackToBuffer = NULL;
	SmackToBufferRect = NULL;
	SmackGoto = NULL;
	SmackColorRemap = NULL;
	SmackColorTrans = NULL;
	SmackFrameRate = NULL;
	SmackGetTrackData = NULL;
	SmackSoundUseDirectSound = NULL;

	if (s_Smacker) {
		FreeLibrary(s_Smacker);
		s_Smacker = NULL;
	}
}

/************************************************************************/
