/************************************************************************/
/* File Name   : sha.h                                                  */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 19th, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : Blizzard's SHA-0 digest algorithm API definition       */
/************************************************************************/

#ifndef __SD_LAWINE_MISC_SHA_H__
#define __SD_LAWINE_MISC_SHA_H__

/************************************************************************/

#include <common.h>

/************************************************************************/

struct SHA_CONTEXT {
	DWORD h0;
	DWORD h1;
	DWORD h2;
	DWORD h3;
	DWORD h4;
};

/************************************************************************/

CAPI extern VOID sha_init(struct SHA_CONTEXT *sha);
CAPI extern VOID sha_update(struct SHA_CONTEXT *sha, VCPTR buf);

/************************************************************************/

#endif	/* __SD_LAWINE_MISC_SHA_H__ */
