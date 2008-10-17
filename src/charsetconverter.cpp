#include "../include/charsetconverter.h"

#include <vector>

CharsetConverter::CharsetConverter()
{
	m_iconv=(iconv_t)-1;
	m_fromcharset="";
	m_tocharset="";	
}

CharsetConverter::CharsetConverter(const std::string &fromcharset, const std::string &tocharset)
{
	SetConversion(fromcharset,tocharset);
}

CharsetConverter::~CharsetConverter()
{
	if(m_iconv!=(iconv_t)-1)
	{
		iconv_close(m_iconv);
	}
}

const bool CharsetConverter::SetConversion(const std::string &fromcharset, const std::string &tocharset)
{
	if(m_iconv!=(iconv_t)-1)
	{
		iconv_close(m_iconv);
		m_iconv=(iconv_t)-1;
	}
	if((m_iconv=iconv_open(tocharset.c_str(),fromcharset.c_str()))!=(iconv_t)-1)
	{
		m_fromcharset=fromcharset;
		m_tocharset=tocharset;
		return true;	
	}
	else
	{
		m_fromcharset="";
		m_tocharset="";
		return false;
	}
}

const bool CharsetConverter::Convert(const std::string &input, std::string &output)
{
	if(m_iconv!=(iconv_t)-1)
	{
		if(input.size()==0)
		{
			return true;
		}

		std::vector<char> invec(input.begin(),input.end());
		std::vector<char> outvec(invec.size()*4,0);
#if defined(_WIN32) || defined(__FreeBSD__)//|| defined(__APPLE__) || defined(__DARWIN__)
		const char *inptr=&invec[0];
#else
		char *inptr=&invec[0];
#endif
		char *outptr=&outvec[0];
		size_t insize=invec.size();
		size_t outsize=outvec.size();

		size_t rval=0;
		
		rval=iconv(m_iconv,&inptr,&insize,&outptr,&outsize);
		
		if(outsize>=0)
		{
			outvec.resize(outptr-&outvec[0]);
			output="";
			output.append(outvec.begin(),outvec.end());
			return true;
		}
		else
		{
			return false;
		}
		
	}
	else
	{
		return false;	
	}
}
