#ifndef _UTF_conversion_
#define _UTF_conversion_

/*
	Thanks to http://www.codeproject.com/KB/string/UtfConverter.aspx for the basic idea
	http://www.unicode.org/Public/PROGRAMS/CVTUTF/
*/

#include <string>
#include <vector>

#include "ConvertUTF.h"

namespace UTFConversion
{

// UTF-8 byte sequence to UTF-16 or UTF-32, depending on size of wchar_t
const bool FromUTF8(const std::vector<std::string::value_type> &utf8string, std::wstring &wcstring);
const bool FromUTF8(const std::string &utf8string, std::wstring &wcstring);
// UTF-16 or UTF-32 to UTF-8 byte sequence
const bool ToUTF8(const std::wstring &wcstring, std::string &utf8string);

}	// namespace

#endif	// _UTF_conversion_
