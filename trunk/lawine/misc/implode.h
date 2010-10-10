/************************************************************************/
/* File Name   : implode.h                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 23rd, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : PKWare DCL implode compression API definition          */
/************************************************************************/

#ifndef __SD_LAWINE_COMPRESS_IMPLODE_H__
#define __SD_LAWINE_COMPRESS_IMPLODE_H__

/************************************************************************/

#include <common.h>

/************************************************************************/

#define IMPLODE_BINARY		0			// Binary compression
#define IMPLODE_ASCII		1			// ASCII compression

#define IMPLODE_DICT_1K		4			// Dictionary size is 1024
#define IMPLODE_DICT_2K		5			// Dictionary size is 2048
#define IMPLODE_DICT_4K		6			// Dictionary size is 4096

/************************************************************************/

CAPI extern BOOL implode(INT type, INT dict, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size);
CAPI extern BOOL explode(VCPTR src, UINT src_size, VPTR dest, UINT *dest_size);

/************************************************************************/

#endif	/* __SD_LAWINE_COMPRESS_IMPLODE_H__ */
