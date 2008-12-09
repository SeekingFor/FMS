#include "../../include/unicode/unicodeformatter.h"
#include "../../include/unicode/utfconversion.h"

std::wstring UnicodeFormatter::m_unicodenewline=L"\n";
wchar_t UnicodeFormatter::m_unicodewhitespace[]={0x0009,0x000A,0x000B,0x000C,0x000D,
												0x0020,0x0085,0x00A0,0x1680,0x180E,
												0x2000,0x2001,0x2002,0x2003,0x2004,
												0x2005,0x2006,0x2007,0x2008,0x2009,
												0x200A,0x200B,0x2029,0x202F,0x205F,
												0x3000,0xFEFF};

const bool UnicodeFormatter::LineWrap(const std::string &utf8input, const int linelength, const std::string &ignorechars, std::string &utf8output)
{
	std::wstring wcstring;
	std::wstring wcignorechars;

	if(UTFConversion::FromUTF8(utf8input,wcstring) && UTFConversion::FromUTF8(ignorechars,wcignorechars))
	{

		std::wstring::size_type currentpos=0;
		std::wstring::size_type lastnewlinepos=0;
		std::wstring::size_type whitespacepos=0;

		while(currentpos+linelength<wcstring.length())
		{
			if(ignorechars.size()==0 || wcstring.find_first_of(wcignorechars,currentpos)!=currentpos)
			{
				lastnewlinepos=wcstring.rfind(m_unicodenewline,currentpos+linelength);
				// newline found within line length - we don't need to wrap
				if(lastnewlinepos!=std::wstring::npos && lastnewlinepos>=currentpos)
				{
					currentpos=lastnewlinepos+1;
				}
				// newline doesn't exist at all - force one in
				else if(lastnewlinepos==std::wstring::npos)
				{
					wcstring.insert(currentpos+linelength,m_unicodenewline);
					currentpos+=linelength+m_unicodenewline.length();
				}
				else
				{
					whitespacepos=wcstring.find_last_of(m_unicodewhitespace,currentpos+linelength);
					// whitespace found within line length - erase whitespace and insert newline
					if(whitespacepos!=std::wstring::npos && whitespacepos>=currentpos)
					{
						wcstring.erase(whitespacepos,1);
						wcstring.insert(whitespacepos,m_unicodenewline);
						currentpos=whitespacepos+m_unicodenewline.length();
					}
					// whitespace not found within line length - force newline at line length
					else
					{
						wcstring.insert(currentpos+linelength,m_unicodenewline);
						currentpos+=linelength+m_unicodenewline.length();
					}
				}
			}
			else
			{
				currentpos=wcstring.find(m_unicodenewline,currentpos+1);
				if(currentpos==std::string::npos)
				{
					currentpos=wcstring.size();
				}
				currentpos++;
			}
		}

		if(UTFConversion::ToUTF8(wcstring,utf8output))
		{
			return true;
		}

	}

	return false;

}
