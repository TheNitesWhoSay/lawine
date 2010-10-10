/************************************************************************/
/* File Name   : idea.h                                                 */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 19th, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : Blizzard's IDEA encryption algorithm API definition    */
/************************************************************************/

#ifndef __SD_LAWINE_MISC_IDEA_H__
#define __SD_LAWINE_MISC_IDEA_H__

/************************************************************************/

#include <common.h>

/************************************************************************/

struct IDEA_KEY {
	WORD data[9][6];
};

/************************************************************************/

CAPI extern VOID idea_decrypt_key(CONST struct IDEA_KEY *ekey, struct IDEA_KEY *dkey);
CAPI extern VOID idea_encrypt(VPTR dest, VCPTR src, CONST struct IDEA_KEY *key);

/************************************************************************/

#endif	/* __SD_LAWINE_MISC_IDEA_H__ */
