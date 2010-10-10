/************************************************************************/
/* File Name   : smk.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 10th, 2009                                         */
/* Module      : Lawine library                                         */
/* Descript    : DSmk class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_SMK_HPP__
#define __SD_LAWINE_DATA_SMK_HPP__

/************************************************************************/

#include <common.h>
#include <file.hpp>
#include <image.hpp>
#include <palette.hpp>
#include "../misc/smack.h"

/************************************************************************/

class DSmk {

public:

	DSmk();
	~DSmk();

	BOOL Load(STRCPTR name);
	VOID Clear(VOID);
	BOOL Play(VOID);
	BOOL Start(INT loop = 0);
	BOOL Stop(VOID);
	BOOL Pause(VOID);
	BOOL Resume(VOID);

	INT GetFrameNum(VOID) CONST;
	INT GetCurFrame(VOID) CONST;
	INT GetRestLoop(VOID) CONST;
	BOOL GetSize(SIZE &size) CONST;
	CONST DImage *GetImage(VOID) CONST;

protected:

	VOID TransferPalette(INT type, BUFCPTR pal);

	BOOL			m_Loop;
	BOOL			m_Play;
	BOOL			m_Pause;
	INT				m_FrameNum;
	DImage			m_Image;
	DPalette		m_Palette;
	DFile			m_File;
	SMACK			*m_Smack;

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_SMK_HPP__ */
