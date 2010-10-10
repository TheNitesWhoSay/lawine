/************************************************************************/
/* File Name   : color.h                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 11st, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : Color relatived API definition                         */
/************************************************************************/

#ifndef __SD_LAWINE_MISC_COLOR_H__
#define __SD_LAWINE_MISC_COLOR_H__

/************************************************************************/

#include <common.h>
#include <image.h>

/************************************************************************/

CAPI extern BOOL CycleColor(PALPTR pal);
CAPI extern BOOL GetUserColor(PALPTR pal, INT user);

/************************************************************************/

#endif	/* __SD_LAWINE_MISC_COLOR_H__ */
