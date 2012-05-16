/************************************************************************/
/* File Name   : adpcm.h                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 5th, 2012                                          */
/* Module      : Lawine library                                         */
/* Descript    : Blizzard ADPCM compression API definition            */
/************************************************************************/

#ifndef __SD_LAWINE_MISC_ADPCM_H__
#define __SD_LAWINE_MISC_ADPCM_H__

/************************************************************************/

#include <common.h>

/************************************************************************/

#define ADPCM_MONO		1
#define ADPCM_STEREO	2

/************************************************************************/

CAPI extern BOOL adpcm_encode(INT type, INT channels, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size);
CAPI extern BOOL adpcm_decode(INT channels, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size);

CAPI extern BOOL adpcm_beta_encode(INT type, INT channels, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size);
CAPI extern BOOL adpcm_beta_decode(INT channels, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size);

/************************************************************************/

#endif	/* __SD_LAWINE_MISC_HUFFMAN_H__ */
