#ifndef _charset_converter_
#define _charset_converter_

#include <iconv.h>
#include <string>

class CharsetConverter
{
public:
	CharsetConverter();
	CharsetConverter(const std::string &fromcharset, const std::string &tocharset);
	~CharsetConverter();
	
	const bool SetConversion(const std::string &fromcharset, const std::string &tocharset);
	
	const bool Convert(const std::string &input, std::string &output);

private:

	iconv_t m_iconv;
	std::string m_fromcharset;
	std::string m_tocharset;

};

#endif	// _charset_converter_
