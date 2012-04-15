/************************************************************************/
/* File Name   : common.c                                               */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 5th, 2009                                          */
/* Module      : Common library                                         */
/* Descript    : Common definations                                     */
/************************************************************************/

#include <common.h>

/************************************************************************/

static CHAR s_Cwd[MAX_PATH + 1];

/************************************************************************/

CAPI STRCPTR DGetCwd(VOID)
{
#ifdef _WIN32
	if (!GetCurrentDirectory(MAX_PATH, s_Cwd))
#else
	if (!getcwd(s_Cwd, MAX_PATH))
#endif
		return NULL;

	s_Cwd[MAX_PATH] = '\0';
	return s_Cwd;
}

/************************************************************************/
