/************************************************************************/
/* File Name   : crc32.h                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 23rd, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : Cyclic redundancy check calculation API definition     */
/************************************************************************/

#ifndef __SD_LAWINE_MISC_CRC32_H__
#define __SD_LAWINE_MISC_CRC32_H__

/************************************************************************/

#include <common.h>

/************************************************************************/

#define crc32_checksum(data, size)	(~crc32_update(data, size, ~0))
#define crc32_start(data, size)		(crc32_update(data, size, ~0))
#define crc32_end(cs)				(~(cs))

/************************************************************************/

CAPI extern DWORD crc32_update(VCPTR data, UINT size, DWORD cs);

/************************************************************************/

#endif	/* __SD_LAWINE_MISC_CRC32_H__ */
