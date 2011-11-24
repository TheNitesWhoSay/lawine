/************************************************************************/
/* File Name   : huffman.h                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 9th, 2011                                          */
/* Module      : Lawine library                                         */
/* Descript    : Blizzard huffman compression API definition            */
/************************************************************************/

#ifndef __SD_LAWINE_MISC_HUFFMAN_H__
#define __SD_LAWINE_MISC_HUFFMAN_H__

/************************************************************************/

#include <common.h>

/************************************************************************/

CAPI extern BOOL huff_encode(INT type, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size);
CAPI extern BOOL huff_decode(VCPTR src, UINT src_size, VPTR dest, UINT *dest_size);

/************************************************************************/

#endif	/* __SD_LAWINE_MISC_HUFFMAN_H__ */
