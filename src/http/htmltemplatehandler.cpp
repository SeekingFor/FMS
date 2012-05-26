#include "../../include/http/htmltemplatehandler.h"

#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

const bool HTMLTemplateHandler::GetSection(const std::string &section, std::string &result, const std::vector<std::string> &ignoredvars) const
{
	std::map<std::string,std::string>::const_iterator seci=m_section.find(section);
	if(seci!=m_section.end())
	{
		result=(*seci).second;
		int nestcount=0;
		while(nestcount++<20 && PerformReplacements(result,m_section,result,ignoredvars)>0)
		{
		}

		return true;
	}
	else
	{
		return false;
	}
}

const bool HTMLTemplateHandler::LoadTemplate(const std::string &templatepath)
{
	m_section.clear();
	FILE *infile=fopen(templatepath.c_str(),"rb");
	if(infile)
	{
		fseek(infile,0,SEEK_END);
		long len=ftell(infile);
		fseek(infile,0,SEEK_SET);
		if(len>0)
		{
			std::vector<unsigned char> data(len,0);
			fread(&data[0],1,data.size(),infile);
			std::string templatestr(data.begin(),data.end());
			
			std::string::size_type beginpos=templatestr.find("<!--[BEGIN ");
			while(beginpos!=std::string::npos)
			{
				std::string::size_type pos2=templatestr.find("]-->",beginpos);
				if(pos2!=std::string::npos)
				{
					std::string sectionname(templatestr.substr(beginpos+11,pos2-(beginpos+11)));
					std::string::size_type endpos=templatestr.find("<!--[END "+sectionname+"]-->",beginpos);
					if(endpos!=std::string::npos)
					{
						m_section[sectionname]=templatestr.substr(beginpos+11+sectionname.size()+4,endpos-(beginpos+11+sectionname.size()+4));
					}
				}
				
				beginpos=templatestr.find("<!--[BEGIN ",beginpos+1);
				
			}
			
		}
		fclose(infile);
		return true;
	}
	else
	{
		return false;	
	}
}

const int HTMLTemplateHandler::PerformReplacements(const std::string &text, const std::map<std::string,std::string> &varmap, std::string &result, const std::vector<std::string> &ignoredvars) const
{
	int replaced=0;
	std::string worktext(text);

	std::string::size_type startpos=worktext.find('[');
	while(startpos!=std::string::npos)
	{
		bool didreplace=false;
		std::string::size_type endpos=worktext.find(']',startpos);
		if(endpos!=std::string::npos)
		{
			std::string section=worktext.substr(startpos+1,endpos-(startpos+1));
			if(std::find(ignoredvars.begin(),ignoredvars.end(),section)==ignoredvars.end())
			{
				std::map<std::string,std::string>::const_iterator vari=varmap.find(section);
				if(vari!=varmap.end())
				{
					worktext.replace(startpos,(endpos-startpos)+1,(*vari).second);
					replaced++;
					didreplace=true;
				}
				else	// section was not in the supplied vars, so grab it from another section if we can
				{
					vari=m_section.find(section);
					if(vari!=m_section.end())
					{
						worktext.replace(startpos,(endpos-startpos)+1,(*vari).second);
						replaced++;
						didreplace=true;
					}
				}
			}
		}

		if(didreplace==true)
		{
			startpos=worktext.find('[',startpos);
		}
		else
		{
			startpos=worktext.find('[',startpos+1);
		}
	}

	result=worktext;

	return replaced;
}

void HTMLTemplateHandler::PerformTranslations(const std::string &text, const StringTranslation &translations, std::string &result) const
{
	std::string worktext(text);
	std::string::size_type startpos=worktext.find("{translation.");
	while(startpos!=std::string::npos)
	{
		std::string::size_type endpos=worktext.find('}',startpos);
		if(endpos!=std::string::npos)
		{
			std::string transname(worktext.substr(startpos+13,endpos-(startpos+13)));
			if(translations.IDExists(transname)==true)
			{
				worktext.replace(startpos,(endpos-startpos)+1,translations.Get(transname));
			}
		}
		startpos=worktext.find("{translation.",startpos+1);
	}
	result=worktext;
}
