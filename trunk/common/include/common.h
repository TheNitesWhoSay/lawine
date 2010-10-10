/************************************************************************/
/* File Name   : common.h                                               */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Nov 27th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : Common definations                                     */
/************************************************************************/

#ifndef __SD_COMMON_COMMON_H__
#define __SD_COMMON_COMMON_H__

/************************************************************************/

#include <const.h>

#ifdef __cplusplus

#include <ctime>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cassert>

#else

#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#endif

/************************************************************************/

#ifdef __cplusplus
#define CAPI			extern "C"
#else
#define CAPI
#endif

/************************************************************************/

#define DCount(arr)			(sizeof(arr) / sizeof((arr)[0]))
#define DVarClr(var)		DMemClr(&(var), sizeof(var))
#define DSwap(x, y)			do { x ^= y; y ^= x; x ^=y; } while (0)
#define DBetween(var, b, e)	((var) >= (b) && (var) < (e))
#define DBoolean(b)			(!!(b))

/************************************************************************/

#ifdef WIN32
#include <win32.h>
#else
#include <posix.h>
#endif

typedef VOID			*VPTR;
typedef CONST VOID		*VCPTR;

typedef CHAR			*STRPTR;
typedef CONST CHAR		*STRCPTR;

typedef BYTE			*BUFPTR;
typedef CONST BYTE		*BUFCPTR;

/************************************************************************/

CAPI extern STRCPTR DGetCwd(VOID);

/************************************************************************/

#endif	/* __SD_COMMON_COMMON_H__ */
