#include "../../include/unicode/utfconversion.h"

#ifdef _WIN32
#include "../../include/pstdint.h"
#endif

namespace UTFConversion
{

const bool FromUTF8(const std::vector<std::string::value_type> &utf8string, std::wstring &wcstring)
{
	if(utf8string.size()==0)
	{
		wcstring.assign(L"");
		return true;
	}

	std::vector<std::wstring::value_type> dest(utf8string.size(),0);		// dest will never be bigger than the input but could be smaller
	
	const UTF8 *sourcestart=reinterpret_cast<const UTF8 *>(&utf8string[0]);
	const UTF8 *sourceend=sourcestart+utf8string.size();
	
	if(sizeof(std::wstring::value_type)==2 && sizeof(UTF16)==2)
	{	
		UTF16 *deststart=reinterpret_cast<UTF16 *>(&dest[0]);
		UTF16 *destend=deststart+dest.size();
		
		ConversionResult rval=ConvertUTF8toUTF16(&sourcestart,sourceend,&deststart,destend,lenientConversion);
		
		if(rval!=conversionOK)
		{
			return false;	
		}
		
		wcstring.assign(dest.begin(),dest.end()-(destend-deststart));
		
	}
	else if(sizeof(std::wstring::value_type)==4 && sizeof(UTF32)==4)
	{
		UTF32 *deststart=reinterpret_cast<UTF32 *>(&dest[0]);
		UTF32 *destend=deststart+dest.size();
		
		ConversionResult rval=ConvertUTF8toUTF32(&sourcestart,sourceend,&deststart,destend,lenientConversion);

		if(rval!=conversionOK)
		{
			return false;
		}
		
		wcstring.assign(dest.begin(),dest.end()-(destend-deststart));
		
	}
	else
	{
		std::vector<uint32_t> dest2(utf8string.size(),0);
		UTF32 *deststart=reinterpret_cast<UTF32 *>(&dest2[0]);
		UTF32 *destend=deststart+dest2.size();

		ConversionResult rval=ConvertUTF8toUTF32(&sourcestart,sourceend,&deststart,destend,lenientConversion);

		if(rval!=conversionOK)
		{
			return false;
		}

		wcstring.assign(dest2.begin(),dest2.end()-(destend-deststart));

	}

	return true;
}

const bool FromUTF8(const std::string &utf8string, std::wstring &wcstring)
{

	if(utf8string.size()>0)
	{
		return FromUTF8(std::vector<std::string::value_type>(utf8string.begin(),utf8string.end()),wcstring);
	}
	else
	{
		wcstring.assign(L"");
		return true;
	}

}

const bool ToUTF8(const std::wstring &wcstring, std::string &utf8string)
{
	if(wcstring.size()==0)
	{
		utf8string.assign("");
		return true;
	}

	std::vector<std::wstring::value_type> source(wcstring.begin(),wcstring.end());

	if(sizeof(std::wstring::value_type)==2 && sizeof(UTF16)==2)
	{
		std::vector<std::string::value_type> dest(wcstring.size()*2,0);
		
		const UTF16 *sourcestart=reinterpret_cast<const UTF16 *>(&source[0]);
		const UTF16 *sourceend=sourcestart+source.size();
		
		UTF8 *deststart=reinterpret_cast<UTF8 *>(&dest[0]);
		UTF8 *destend=deststart+dest.size();
		
		ConversionResult rval=ConvertUTF16toUTF8(&sourcestart,sourceend,&deststart,destend,lenientConversion);
		
		if(rval!=conversionOK)
		{
			return false;
		}
		
		utf8string.assign(dest.begin(),dest.end()-(destend-deststart));
		
	}
	else if(sizeof(std::wstring::value_type)==4 && sizeof(UTF32)==4)
	{
		std::vector<std::string::value_type> dest(wcstring.size()*4,0);
		
		const UTF32 *sourcestart=reinterpret_cast<const UTF32 *>(&source[0]);
		const UTF32 *sourceend=sourcestart+source.size();
		
		UTF8 *deststart=reinterpret_cast<UTF8 *>(&dest[0]);
		UTF8 *destend=deststart+dest.size();
		
		ConversionResult rval=ConvertUTF32toUTF8(&sourcestart,sourceend,&deststart,destend,lenientConversion);
		
		if(rval!=conversionOK)
		{
			return false;
		}
		
		utf8string.assign(dest.begin(),dest.end()-(destend-deststart));
		
	}
	else
	{
		std::vector<uint32_t> source2(wcstring.begin(),wcstring.end());
		std::vector<std::string::value_type> dest(wcstring.size()*sizeof(std::wstring::value_type),0);
		
		const UTF32 *sourcestart=reinterpret_cast<const UTF32 *>(&source2[0]);
		const UTF32 *sourceend=sourcestart+source2.size();

		UTF8 *deststart=reinterpret_cast<UTF8 *>(&dest[0]);
		UTF8 *destend=deststart+dest.size();

		ConversionResult rval=ConvertUTF32toUTF8(&sourcestart,sourceend,&deststart,destend,lenientConversion);

		if(rval!=conversionOK)
		{
			return false;
		}

		utf8string.assign(dest.begin(),dest.end()-(destend-deststart));

	}

	return true;
}

}	// namespace
