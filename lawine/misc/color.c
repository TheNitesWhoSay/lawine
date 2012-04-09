/************************************************************************/
/* File Name   : color.c                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 11st, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : Color relatived API implementation                     */
/************************************************************************/

#include "color.h"

/************************************************************************/

#define CYCLING_SECTION_NUM		3

/************************************************************************/

struct CYCLING_SECTION {
	BYTE begin_entry;
	BYTE end_entry;
};

/************************************************************************/

CONST struct CYCLING_SECTION CYCLING[CYCLING_SECTION_NUM] = {
	{ 0x01, 0x06 }, { 0x07, 0x0d }, { 0xf8, 0xfe },
};

/************************************************************************/

BOOL cycle_color(PALPTR pal)
{
	INT i;
	PALPTR p;
	COLOR color;

	if (!pal)
		return FALSE;

	for (i = 0; i < CYCLING_SECTION_NUM; i++) {
		color = pal[CYCLING[i].end_entry];
		p = pal + CYCLING[i].begin_entry;
		DMemMov(p + 1, p, (CYCLING[i].end_entry - CYCLING[i].begin_entry) * sizeof(COLOR));
		*p = color;
	}

	return TRUE;
}

BOOL get_user_color(PALPTR pal, INT user)
{
	// TODO:
	return FALSE;
}

/************************************************************************/

/************************************************************************/
