/************************************************************************/
/* File Name   : win32.h                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Nov 27th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : Common definations for WIN32                           */
/************************************************************************/

#ifndef __SD_COMMON_WIN32_H__
#define __SD_COMMON_WIN32_H__

/************************************************************************/

#pragma warning(disable:4996)

/************************************************************************/

//#define _CRT_SECURE_NO_WARNINGS

/************************************************************************/

#include <windows.h>
#include <crtdbg.h>

/************************************************************************/

typedef LONGLONG				LLONG;
typedef DWORDLONG				QWORD;

/************************************************************************/

typedef enum {
	SM_BEGIN	= FILE_BEGIN,
	SM_CURRENT	= FILE_CURRENT,
	SM_END		= FILE_END,
} SEEK_MODE;

#define ERROR_SIZE		INVALID_FILE_SIZE
#define ERROR_POS		INVALID_SET_FILE_POINTER

/************************************************************************/

#define DAlloc					malloc
#define DFree					free
#define DRealloc				realloc

#define DMemCmp					memcmp
#define DMemChr					memchr
#define DMemCpy					CopyMemory
#define DMemSet(m, c, s)		FillMemory(m, s, c)
#define DMemMov					MoveMemory
#define DMemClr					ZeroMemory

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
#define DStrCmpI				_stricmp
#define DStrCmpN				strncmp
#define DStrCmpNI				_strnicmp
#define DStrStr					strstr
#define DSprintf				_snprintf
#define DSScanf					sscanf_s
#define DVSprintf				_vsnprintf

#define DMin					min
#define DMax					max
#define DRandSeed				srand
#define DInitRand()				srand((UINT)time(NULL))
#define DRandom					rand
#define DTime()					time(NULL)
#define DClock					clock

#define DLoc2Lang				LANGIDFROMLCID

#define DMakeDWord				MAKELONG
#define DLoWord					LOWORD
#define DHiWord					HIWORD
#define DMakeWord				MAKEWORD
#define DLoByte					LOBYTE
#define DHiByte					HIBYTE

#ifdef NDEBUG
#define DAssert(x)				((VOID)0)
#define DVerify(x)				(x)
#else
#define DAssert(x)				_ASSERT(x)
#define DVerify(x)				_ASSERT(x)
#endif

/************************************************************************/

#endif	/* __SD_COMMON_WIN32_H__ */
