/************************************************************************/
/* File Name   : posix.h                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Nov 27th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : Common definations for POSIX                           */
/************************************************************************/

#ifndef __SD_COMMON_POSIX_H__
#define __SD_COMMON_POSIX_H__

/************************************************************************/

#include <unistd.h>
#include <sys/types.h>

/************************************************************************/

#define VOID					void
#define CONST					CONST
#define TRUE					true
#define FALSE					false

#define interface				struct

/************************************************************************/

typedef int						INT;
typedef char					CHAR;
typedef short					SHORT;
typedef long					LONG;
typedef long long				LLONG;

typedef unsigned int			UINT;
typedef unsigned char			BYTE;
typedef unsigned short			WORD;
typedef unsigned long			DWORD;
typedef unsigned long long		QWORD;

typedef float					FLOAT;
typedef double					DOUBLE;

typedef bool					BOOL;
typedef unsigned short			LANGID;
typedef unsigned long			LCID;

typedef void					*HANDLE;

/************************************************************************/

typedef struct {
	LONG x;
	LONG y;
} POINT;

typedef struct {
	LONG cx;
	LONG cy;
} SIZE;

typedef struct {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT;

/************************************************************************/

enum SEEK_MODE {
	SM_BEGIN	= SEEK_SET,
	SM_CURRENT	= SEEK_CUR,
	SM_END		= SEEK_END,
};

#define ERROR_SIZE		0xffffffffUL
#define ERROR_POS		0xffffffffUL

#define LANG_NEUTRAL	0x00

/************************************************************************/

#define DAlloc					malloc
#define DFree					free
#define DRealloc				realloc

#define DMemCmp					memcmp
#define DMemChr					memchr
#define DMemCpy					memcpy
#define DMemSet					memset
#define DMemMov					memmove
#define DMemClr(mem, size)		memset(mem, 0, size)

#define DIsUpper				isupper
#define DIsLower				islower
#define DIsAlpha				isalpha
#define DIsDigit				isdigit
#define DIsHex					isxdigit
#define DIsAlNum				isalnum
#define DIsBlank				isspace
#define DToUpper				toupper
#define DToLower				tolower

#define DStrCat					strncat
#define DStrLen					strlen
#define DStrCpy					strcpy
#define DStrCpyN				strncpy
#define DStrChr					strchr
#define DStrRChr				strrchr
#define DStrCmp					strcmp
#define DStrCmpI				stricmp
#define DStrCmpN				strncmp
#define DStrCmpNI				strnicmp
#define DStrStr					strstr
#define DSprintf				snprintf
#define DSScanf					sscanf
#define DVSprintf				vsnprintf

#define DMin(a, b)				(((a) < (b)) ? (a) : (b))
#define DMax(a, b)				(((a) > (b)) ? (a) : (b))
#define DRandSeed				srand
#define DRandom					rand
#define DTime()					time(NULL)
#define DClock					clock

#define DLoc2Lang(loc)			((LANGID)(loc))

#define DMakeDWord(lo, hi)		((WORD)(lo) | ((DWORD)(hi) << 16))
#define DLoWord(dw)				((WORD)(dw))
#define DHiWord(dw)				((WORD)((DWORD)(dw) >> 16))
#define DMakeWord(lo, hi)		((BYTE)(lo) | ((WORD)(hi) << 8))
#define DLoByte(w)				((BYTE)(w))
#define DHiByte(w)				((BYTE)((WORD)(w) >> 8))

#define DAssert					assert
#ifdef NDEBUG
#define DVerify(x)				x
#else
#define DVerify(x)				assert(x)
#endif

/************************************************************************/

#endif	/* __SD_COMMON_POSIX_H__ */
