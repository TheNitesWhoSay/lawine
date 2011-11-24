/************************************************************************/
/* File Name   : smack.h                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Dec 24th, 2008                                         */
/* Module      : Lawine library                                         */
/* Descript    : Smacker library definition                             */
/************************************************************************/

#ifndef __SD_LAWINE_MISC_SMACK_H__
#define __SD_LAWINE_MISC_SMACK_H__

/************************************************************************/

#include <common.h>
#include <dsound.h>

/************************************************************************/

/* Flags for SmackUseMMX */
enum SMACK_MMX_FLAG {
	SMACK_MMX_OFF,
	SMACK_MMX_ON,	
	SMACK_MMX_QUERY_CUR,
};

/* Flags for SmackOpen */
#define SMACK_NEEDPAN			0x00000020U		// Will be setting the pan
#define SMACK_NEEDVOLUME		0x00000040U		// Will be setting the volume
#define SMACK_FRAMERATE			0x00000080U		// Override freq (call SmackFrameRate first)
#define SMACK_LOADEXTRA			0x00000100U		// Load the extra buffer during SmackOpen
#define SMACK_PRELOADALL		0x00000200U		// Preload the entire animation
#define SMACK_NOSKIP			0x00000400U		// Don't skip frames if falling behind
#define SMACK_FILEHANDLE		0x00001000U		// Use when passing in a file handle
#define SMACK_TRACK1	 		0x00002000U		// Play audio track 1
#define SMACK_TRACK2	 		0x00004000U		// Play audio track 2
#define SMACK_TRACK3	 		0x00008000U		// Play audio track 3
#define SMACK_TRACK4	 		0x00010000U		// Play audio track 4
#define SMACK_TRACK5	 		0x00020000U		// Play audio track 5
#define SMACK_TRACK6	 		0x00040000U		// Play audio track 6
#define SMACK_TRACK7	 		0x00080000U		// Play audio track 7
#define SMACK_TRACKS			(SMACK_TRACK1 | SMACK_TRACK2 | SMACK_TRACK3 | SMACK_TRACK4 | SMACK_TRACK5 | SMACK_TRACK6 | SMACK_TRACK7)
#define SMACK_AUTOEXTRA			((VPTR)0xffffffffUL)	// NOT A FLAG! - Use as extrabuf param 

/* Flags for SmackToBuffer */
#define SMACK_BUFFMT_REVERSE	0x00000001U		// Reverse image view
#define SMACK_BUFFMT_256		0x00000000U		// Transfer as 256-color indexed bitmap with palette
#define SMACK_BUFFMT_R5G5B5		0x80000000U		// Transfer as X1R5G5B5 16-bits bitmap
#define SMACK_BUFFMT_R5G6B5		0xc0000000U		// Transfer as R5G6B5 16-bits bitmap

/************************************************************************/

typedef struct {
	CHAR version[4];			// SMK2 only right now
	UINT width;					// Width (1 based, 640 for example)
	UINT height;				// Height (1 based, 480 for example)
	UINT frame_num;				// Number of frames (1 based, 100 = 100 frames)
	UINT frame_rate;			// Frame Rate (ms per frame)
	DWORD smacker_type;			// bit 0 set = ring frame
	DWORD largest_in_track[7];	// Largest single size for each track
	UINT table_size;			// Size of the init tables
	UINT code_size;				// Compression info	
	UINT ab_size;				// ditto
	UINT detail_size;			// ditto
	UINT type_size;				// ditto
	DWORD track_type[7];		// high byte=0x80-Comp,0x40-PCM data,0x20-16 bit,0x10-stereo
	DWORD extra;				// extra value (should be zero)
	INT new_palette;			// set to one if the palette changed
	BYTE palette[772];			// palette data
	INT pal_type;				// type of palette
	UINT frame_no;				// Frame Number to be displayed
	UINT frame_size;			// The current frame's size in bytes
	UINT snd_size;				// The current frame sound tracks' size in bytes
	INT last_rect_x;			// Rect set in from SmackToBufferRect (X coord)
	INT last_rect_y;			// Rect set in from SmackToBufferRect (Y coord)
	INT last_rect_w;			// Rect set in from SmackToBufferRect (Width)
	INT last_rect_h;			// Rect set in from SmackToBufferRect (Height)
	UINT open_flags;			// flags used on open
	UINT left_offset;			// Left Offset used in SmackTo
	UINT top_offset;			// Top Offset used in SmackTo
	UINT largest_frame_size;	// Largest frame size
	UINT highest_1sec_rate;		// Highest 1 sec data rate
	UINT highest_1sec_frame;	// Highest 1 sec data rate starting frame
	INT read_error;				// Set to non-zero if a read error has ocurred
	DWORD addr32;				// translated address for 16 bit interface
} SMACK;

/************************************************************************/

CAPI extern DWORD (WINAPI *SmackUseMMX)(DWORD flag);						// 0 = off, 1 = on, 2 = query current
CAPI extern SMACK *(WINAPI *SmackOpen)(LPCSTR name, UINT flags, VPTR extra_buf);
CAPI extern INT (WINAPI *SmackDoFrame)(SMACK *smk);
CAPI extern VOID (WINAPI *SmackNextFrame)(SMACK *smk);
CAPI extern INT (WINAPI *SmackWait)(SMACK *smk);
CAPI extern VOID (WINAPI *SmackClose)(SMACK *smk);
CAPI extern VOID (WINAPI *SmackVolumePan)(SMACK *smk, UINT track_flag, UINT volume, UINT pan);
CAPI extern INT (WINAPI *SmackSoundInTrack)(SMACK *smk, UINT track_flags);
CAPI extern INT (WINAPI *SmackSoundOnOff)(SMACK *smk, BOOL on);
CAPI extern VOID (WINAPI *SmackToBuffer)(SMACK *smk, INT left, INT top, UINT pitch, INT dest_height, VPTR buf, UINT flags);
CAPI extern BOOL (WINAPI *SmackToBufferRect)(SMACK *smk, DWORD smack_surface);
CAPI extern VOID (WINAPI *SmackGoto)(SMACK *smk, INT frame);
CAPI extern VOID (WINAPI *SmackColorRemap)(SMACK *smk, VPTR remap_pal, UINT num_colors, INT pal_type);
CAPI extern VOID (WINAPI *SmackColorTrans)(SMACK *smk, VCPTR trans);
CAPI extern VOID (WINAPI *SmackFrameRate)(DWORD force_rate);
CAPI extern INT (WINAPI *SmackGetTrackData)(SMACK *smk, VPTR dest, UINT track_flag);
CAPI extern BYTE (WINAPI *SmackSoundUseDirectSound)(LPDIRECTSOUND ds);		// NULL = Create

/************************************************************************/

CAPI extern BOOL init_smacker(LPDIRECTSOUND ds);
CAPI extern VOID exit_smacker(VOID);

/************************************************************************/

#endif	/* __SD_LAWINE_MISC_SMACK_H__ */
