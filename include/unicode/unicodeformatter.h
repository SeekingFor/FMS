#ifndef _unicode_formatter_
#define _unicode_formatter_

#include <string>

class UnicodeFormatter
{
public:

	static const bool LineWrap(const std::string &utf8input, const int linelength, const std::string &ignorechars, std::string &utf8output);

private:

	static std::wstring m_unicodenewline;
	static std::wstring::value_type m_unicodewhitespace[];

};

#endif	// _unicode_formatter_
