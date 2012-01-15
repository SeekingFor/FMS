#include "../../include/http/emoticonreplacer.h"
#include "../../include/stringfunctions.h"
#include "../../include/unicode/unicodestring.h"

//#include <Poco/RegularExpression.h>

#include <algorithm>
#include <string>

EmoticonReplacer::EmoticonReplacer()
{
}

EmoticonReplacer::EmoticonReplacer(const std::string &imagepath):m_imagepath(imagepath)
{
	Initialize(imagepath);
}

void EmoticonReplacer::Initialize(const std::string &imagepath)
{
	m_imagepath=imagepath;

	m_emoticons.reserve(110);
	m_emoticons.push_back(std::pair<std::string,std::string>(":)","1.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-)","1.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":(","2.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-(","2.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(";)","3.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(";-)","3.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":D","4.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-D","4.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(";;)","5.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-/","6.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":/","6.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":x","7.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":X","7.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-x","7.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-X","7.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":\">","8.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":p","9.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-p","9.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":P","9.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-P","9.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":*","10.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-*","10.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":O","11.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":o","11.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-O","11.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-o","11.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("X-(","12.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":>","13.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":->","13.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("B)","14.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("B-)","14.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":s","15.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":S","15.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-s","15.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-S","15.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(">:)","16.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":((","17.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":))","18.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":|","19.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-|","19.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("/:)","20.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("O:)","21.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("o:)","21.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-B","22.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("=;","23.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("I-)","24.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("|-)","24.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("8-|","25.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-&","26.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-$","27.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("[-(","28.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":o)","29.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("8-}","30.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("(:|","31.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("=P~","32.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-?","33.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("#-o","34.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("#-O","34.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("=D>","35.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":@)","36.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("3:-O","37.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":(|)","38.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("~:>","39.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("@};-","40.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("%%-","41.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("**==","42.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("(~~)","43.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("~o)","44.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("*-:)","45.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("8-X","46.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("=:)","47.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(">-)","48.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-L","49.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("<):)","50.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("[-o","51.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("<@-)","52.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("$-)","53.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-\"","54.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":^o","55.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("b-(","56.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":)>-","57.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("[-X","58.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("\\:D/","59.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(">:D<","60.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("#:-S","61.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("=((","62.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("=))","63.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("L-)","64.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("<:-P","65.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-w","66.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-<","67.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(">:P","68.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(">:/","69.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(";))","70.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("^:)^","71.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>(":-j","72.gif"));
	m_emoticons.push_back(std::pair<std::string,std::string>("(*)","73.gif"));

	std::vector<std::pair<std::string,std::string> >::size_type i,j,count;
	// replace < and > with html encoded variants and push onto end of vector
	count=m_emoticons.size();
	for(i=0; i<count; i++)
	{
		std::string temp=StringFunctions::Replace(m_emoticons[i].first,">","&gt;");
		temp=StringFunctions::Replace(temp,"<","&lt;");
		if(temp!=m_emoticons[i].first)
		{
			m_emoticons.push_back(std::pair<std::string,std::string>(temp,m_emoticons[i].second));
		}
	}

	// move strings that contain substrings to the front of the vector
	for(i=0; i<m_emoticons.size(); i++)
	{
		for(j=i+1; j<m_emoticons.size(); j++)
		{
			if(m_emoticons[j].first.find(m_emoticons[i].first)!=std::string::npos)
			{
				(std::swap)(m_emoticons[i],m_emoticons[j]);
				j=i+1;
			}
		}
	}

}

const std::string EmoticonReplacer::Replace(const std::string &message) const
{/*
	std::string output(message);
	for(std::vector<std::pair<std::string,std::string> >::const_iterator i=m_emoticons.begin(); i!=m_emoticons.end(); i++)
	{
		try
		{
			// we must put \ in front of every non alphanumeric char of the emoticon in case it is a special regex char
			std::string emoticon("");
			for(std::string::const_iterator j=(*i).first.begin(); j!=(*i).first.end(); j++)
			{
				if(!((*j)>='0' && (*j)<='9') && !((*j)>='A' && (*j)<='Z') && !((*j)>='a' && (*j)<='z') && !((*j)=='>' || (*j)=='<'))
				{
					emoticon+="\\";
				}
				emoticon+=(*j);
			}
			// starts with whitespace or new line - emoticon - ends with whitespace or end of line
			Poco::RegularExpression re("(\\s|^)"+emoticon+"(\\s|$)");
			Poco::RegularExpression::Match match;
			re.match(output,match);
			while(match.offset!=std::string::npos)
			{
				std::string::size_type pos=output.find((*i).first,match.offset);
				if(pos!=std::string::npos)
				{
					std::string image("<img src=\""+m_imagepath+(*i).second+"\">");
					output.replace(pos,(*i).first.size(),image);
				}
				re.match(output,match.offset+1,match);
			}
		}
		catch(...)
		{

		}
	}
	return output;*/
	UnicodeString output(message);
	output.Widen();
	UnicodeString::wsize_type pos=std::wstring::npos;
	//std::string output(message);
	//std::string::size_type pos=std::string::npos;
	for(std::vector<std::pair<std::string,std::string> >::const_iterator i=m_emoticons.begin(); i!=m_emoticons.end(); i++)
	{
		pos=output.Find((*i).first);
		//pos=output.find((*i).first);
		//while(pos!=std::string::npos)
		while(pos!=UnicodeString::wnpos)
		{
			//std::string image("<img src=\""+m_imagepath+(*i).second+"\">");
			//output.replace(pos,(*i).first.size(),image);
			//pos=output.find((*i).first,pos+1);
			if((pos==0 || (pos>0 && UnicodeString::IsWhitespace(output[pos-1])==true)) && ((pos+(*i).first.size())==output.Size() || ((pos+(*i).first.size())<output.Size() && UnicodeString::IsWhitespace(output[pos+(*i).first.size()]))))
			{
				UnicodeString image("<img src=\""+m_imagepath+(*i).second+"\">");
				output.Replace(pos,(*i).first.size(),image);
			}
			pos=output.Find((*i).first,pos+1);
		}
	}
	return output.NarrowString();
}
