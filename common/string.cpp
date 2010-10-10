/************************************************************************/
/* File Name   : string.cpp                                             */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Nov 29th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DString class implementation                           */
/************************************************************************/

#include <string.hpp>

/************************************************************************/

CONST STRCPTR EMPTY_STRING = "";

/************************************************************************/

static CHAR s_ErrorAt = '\0';
static CHAR s_FmtBuf[1024];

/************************************************************************/

DString::DString()
{

}

DString::DString(CONST DString &s) : m_String(s)
{

}

DString::DString(STRCPTR s, INT len /* = -1 */)
{
	if (len < 0)
		m_String.assign(s);
	else
		m_String.assign(s, len);
}

DString::DString(CHAR ch, INT count /* = 1 */)
{
	if (ch > 0)
		m_String.assign(count, ch);
}

DString::DString(CONST Core_String &cs)
{
	m_String = cs;
}

CHAR DString::At(INT pos) CONST
{
	if (!DBetween(pos, 0, static_cast<INT>(m_String.length())))
		return '\0';

	return m_String.at(pos);
}

CHAR &DString::At(INT pos)
{
	if (!DBetween(pos, 0, static_cast<INT>(m_String.length()))) {
		DAssert(FALSE);
		return s_ErrorAt;
	}

	return m_String.at(pos);
}

INT DString::Length(VOID) CONST
{
	return static_cast<INT>(m_String.length());
}

BOOL DString::Empty(VOID) CONST
{
	return m_String.empty();
}

DString DString::Left(INT len) CONST
{
	INT max_len = static_cast<INT>(m_String.length());
	if (len > max_len)
		len = max_len;
	else
		return EMPTY_STRING;

	return m_String.substr(0, len);
}

DString DString::Right(INT len) CONST
{
	INT max_len = static_cast<INT>(m_String.length());
	if (len > max_len)
		len = max_len;
	else
		return EMPTY_STRING;

	return m_String.substr(max_len - len, len);
}

DString DString::Middle(INT pos, INT len /* = -1 */) CONST
{
	INT max_len = static_cast<INT>(m_String.length());

	if (!DBetween(pos, 0, max_len))
		return EMPTY_STRING;

	if (len > max_len - pos)
		len = max_len - pos;

	return m_String.substr(pos, len);
}

INT DString::Compare(STRCPTR s, INT len /* = -1 */) CONST
{
	if (!s)
		s = EMPTY_STRING;

	return m_String.compare(0, len, s);
}

INT DString::CompareI(STRCPTR s, INT len /* = -1 */) CONST
{
	if (!s)
		s = EMPTY_STRING;

	return DStrCmpNI(m_String.c_str(), s, len);
}

INT DString::AsDec(VOID) CONST
{
	return ::atoi(m_String.c_str());
}

UINT DString::AsUDec(VOID) CONST
{
	return static_cast<UINT>(::atoi(m_String.c_str()));
}

UINT DString::AsHex(VOID) CONST
{
	UINT hex = 0U;

	DSScanf(m_String.c_str(), "%x", &hex);
	return hex;
}

UINT DString::AsOct(VOID) CONST
{
	UINT oct = 0U;

	DSScanf(m_String.c_str(), "%o", &oct);
	return oct;
}

UINT DString::AsBin(VOID) CONST
{
	UINT bin = 0U;
	STRCPTR p = m_String.c_str();

	while (*p == '0' || *p == '1') {
		bin <<= 1;
		if (*p == '1')
			bin |= 1;
		p++;
	}

	return bin;
}

FLOAT DString::AsFloat(VOID) CONST
{
	return static_cast<FLOAT>(::atof(m_String.c_str()));
}

DOUBLE DString::AsDouble(VOID) CONST
{
	return ::atof(m_String.c_str());
}

INT DString::Find(STRCPTR s, INT pos /* = 0 */, INT len /* = -1 */) CONST
{
	if (!s)
		s = EMPTY_STRING;

	return static_cast<INT>(m_String.find(s, pos, len));
}

INT DString::Find(CHAR ch, INT pos /* = 0 */) CONST
{
	return static_cast<INT>(m_String.find(ch, pos));
}

INT DString::FindR(STRCPTR s, INT pos /* = -1 */, INT len /* = -1 */) CONST
{
	if (!s)
		s = EMPTY_STRING;

	return static_cast<INT>(m_String.rfind(s, pos, len));
}

INT DString::FindR(CHAR ch, INT pos /* = -1 */) CONST
{
	return static_cast<INT>(m_String.rfind(ch, pos));
}

DString &DString::Assign(STRCPTR s)
{
	if (!s)
		m_String.clear();
	else
		m_String.assign(s);

	return *this;
}

DString &DString::Clear(VOID)
{
	m_String.clear();
	return *this;
}

DString &DString::Append(STRCPTR s)
{
	if (s)
		m_String.append(s);

	return *this;
}

DString &DString::Insert(INT pos, CHAR ch, INT count /* = 1 */)
{
	if (!DBetween(pos, 0, static_cast<INT>(m_String.length())))
		return *this;

	if (count <= 0)
		return *this;

	m_String.insert(pos, count, ch);
	return *this;
}

DString &DString::Insert(INT pos, STRCPTR s, INT len /* = -1 */)
{
	if (!s)
		return *this;

	if (!DBetween(pos, 0, static_cast<INT>(m_String.length())))
		return *this;

	m_String.insert(pos, s, len);
	return *this;
}

DString &DString::ToUpper(VOID)
{
	Core_String::iterator it;
	for (it = m_String.begin(); it != m_String.end(); ++it)
		*it = DToUpper(*it);

	return *this;
}

DString &DString::ToLower(VOID)
{
	Core_String::iterator it;
	for (it = m_String.begin(); it != m_String.end(); ++it)
		*it = DToLower(*it);

	return *this;
}

DString &DString::Format(STRCPTR fmt, ...)
{
	if (!fmt)
		return *this;

	va_list arg;
	INT count;
	CHAR *buf = s_FmtBuf;

	va_start(arg, fmt);
	INT size = DCount(s_FmtBuf);
	count = DVSprintf(buf, size, fmt, arg);
	va_end(arg);

	if (count <= 0)
		return *this;

	if (count >= size) {
		buf = NULL;
		do {
			size <<= 1;
			DRealloc(buf, size);
			va_start(arg, fmt);
			count = DVSprintf(buf, size, fmt, arg);
			va_end(arg);
		} while (count >= size);
	}

	Assign(buf);

	if (count <= DCount(s_FmtBuf))
		DVarClr(s_FmtBuf);
	else
		DFree(buf);

	return *this;
}

DString::operator STRCPTR () CONST
{
	return m_String.c_str();
}

CHAR DString::operator [] (INT i) CONST
{
	return At(i);
}

CHAR &DString::operator [] (INT i)
{
	return At(i);
}

CHAR DString::operator * () CONST
{
	return At(0);
}

DString DString::operator + (STRCPTR s) CONST
{
	Core_String cs = *this;

	if (s)
		cs.append(s);

	return cs.c_str();
}

BOOL DString::operator == (STRCPTR s)
{
	return Compare(s) == 0;
}

BOOL DString::operator != (STRCPTR s)
{
	return Compare(s) != 0;
}

BOOL DString::operator > (STRCPTR s)
{
	return Compare(s) > 0;
}

BOOL DString::operator >= (STRCPTR s)
{
	return Compare(s) >= 0;
}

BOOL DString::operator < (STRCPTR s)
{
	return Compare(s) < 0;
}

BOOL DString::operator <= (STRCPTR s)
{
	return Compare(s) <= 0;
}

DString &DString::operator = (STRCPTR s)
{
	return Assign(s);
}

DString &DString::operator += (STRCPTR s)
{
	return Append(s);
}

/************************************************************************/
