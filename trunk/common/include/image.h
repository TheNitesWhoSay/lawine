/************************************************************************/
/* File Name   : image.h                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : April 13rd, 2010                                       */
/* Module      : Common library                                         */
/* Descript    : Image data structure definition                        */
/************************************************************************/

#ifndef __SD_COMMON_IMAGE_H__
#define __SD_COMMON_IMAGE_H__

/************************************************************************/

#include <common.h>

/************************************************************************/

#define D_COLOR_NUM			256
#define D_COLOR_DEPTH		8

/************************************************************************/

typedef struct {
	BUFPTR	data;
	UINT	pitch;
	SIZE	size;
} IMAGE;

typedef struct {
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE reserved;
} COLOR;

typedef IMAGE			*IMGPTR;
typedef CONST IMAGE		*IMGCPTR;

typedef COLOR			*CLRPTR;
typedef CONST COLOR		*CLRCPTR;

typedef CLRPTR			PALPTR;
typedef CLRCPTR			PALCPTR;

/************************************************************************/

#endif	/* __SD_COMMON_IMAGE_H__ */
