/************************************************************************/
/* File Name   : string.hpp                                             */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Nov 29th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DString class declaration                              */
/************************************************************************/

#ifndef __SD_COMMON_STRING_HPP__
#define __SD_COMMON_STRING_HPP__

/************************************************************************/

#include <string>
#include <common.h>

/************************************************************************/

class DString {

protected:

	typedef std::string	CoreString;

public:

	DString();
	DString(CONST DString &s);
	DString(STRCPTR s, INT len = -1);
	DString(CHAR ch, INT count = 1);

	CHAR At(INT pos) CONST;
	CHAR &At(INT pos);

	INT Length(VOID) CONST;
	BOOL Empty(VOID) CONST;

	DString Left(INT len) CONST;
	DString Right(INT len) CONST;
	DString Middle(INT pos, INT len = -1) CONST;

	INT Compare(STRCPTR s, INT len = -1) CONST;
	INT CompareI(STRCPTR s, INT len = -1) CONST;

	INT AsDec(VOID) CONST;
	UINT AsUDec(VOID) CONST;
	UINT AsHex(VOID) CONST;
	UINT AsOct(VOID) CONST;
	UINT AsBin(VOID) CONST;
	FLOAT AsFloat(VOID) CONST;
	DOUBLE AsDouble(VOID) CONST;

	INT Find(STRCPTR s, INT pos = 0, INT len = -1) CONST;
	INT Find(CHAR ch, INT pos = 0) CONST;
	INT FindR(STRCPTR s, INT pos = -1, INT len = -1) CONST;
	INT FindR(CHAR ch, INT pos = -1) CONST;

	DString &Assign(STRCPTR s);
	DString &Clear(VOID);
	DString &Append(STRCPTR s);
	DString &Insert(INT pos, CHAR ch, INT count = 1);
	DString &Insert(INT pos, STRCPTR s, INT len = -1);

	DString &ToUpper(VOID);
	DString &ToLower(VOID);

	DString &Format(STRCPTR fmt, ...);
	DString &AppendFormat(STRCPTR fmt, ...);

	operator STRCPTR () CONST;

	CHAR operator [] (INT i) CONST;
	CHAR &operator [] (INT i);

	CHAR operator * () CONST;
	DString operator + (STRCPTR s) CONST;
	DString operator + (CONST DString &s) CONST;

	BOOL operator == (STRCPTR s);
	BOOL operator != (STRCPTR s);
	BOOL operator > (STRCPTR s);
	BOOL operator >= (STRCPTR s);
	BOOL operator < (STRCPTR s);
	BOOL operator <= (STRCPTR s);

	DString &operator = (STRCPTR s);
	DString &operator += (STRCPTR s);

protected:

	DString(CONST CoreString &cs);

	CoreString	m_String;

};

/************************************************************************/

#endif	/* __SD_COMMON_STRING_HPP__ */
