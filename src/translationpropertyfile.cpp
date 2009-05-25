#include "../include/translationpropertyfile.h"

#include <Poco/FileStream.h>
#include <Poco/LineEndingConverter.h>
#include <cctype>

void TranslationPropertyFile::GetKeys(std::vector<std::string> &resultkeys) const
{
	GetSubKeys("",resultkeys);
}

void TranslationPropertyFile::GetSubKeys(const std::string &prefix, std::vector<std::string> &resultkeys) const
{
	std::vector<std::string> topkeys;
	keys(prefix,topkeys);

	for(std::vector<std::string>::const_iterator i=topkeys.begin(); i!=topkeys.end(); i++)
	{
		std::string newid("");
		if(prefix!="")
		{
			newid=prefix+"."+(*i);
		}
		else
		{
			newid=(*i);
		}
		if(hasProperty(newid)==true)
		{
			resultkeys.push_back(newid);
		}
		else
		{
			GetSubKeys(newid,resultkeys);
		}
	}
}

void TranslationPropertyFile::load(std::istream& istr)
{
	clear();
	while (!istr.eof())
	{
		parseLine(istr);
	}
}

	
void TranslationPropertyFile::load(const std::string& path)
{
	Poco::FileInputStream istr(path);
	if (istr.good())
		load(istr);
	else
		throw Poco::OpenFileException(path);
}


void TranslationPropertyFile::save(std::ostream& ostr) const
{
	MapConfiguration::iterator it = begin();
	MapConfiguration::iterator ed = end();
	while (it != ed)
	{
		ostr << it->first << "= " << it->second << "\n";
		++it;
	}
}


void TranslationPropertyFile::save(const std::string& path) const
{
	Poco::FileOutputStream ostr(path);
	if (ostr.good())
	{
		Poco::OutputLineEndingConverter lec(ostr);
		save(lec);
		lec.flush();
		ostr.flush();
		if (!ostr.good()) throw Poco::WriteFileException(path);
	}
	else throw Poco::CreateFileException(path);
}

void TranslationPropertyFile::parseLine(std::istream& istr)
{
	static const int eof = std::char_traits<char>::eof(); 

	int c = istr.get();
	while (c != eof && std::isspace(c)) c = istr.get();
	if (c != eof)
	{
		if (c == '#' || c == '!')
		{
			while (c != eof && c != '\n' && c != '\r') c = istr.get();
		}
		else
		{
			std::string key;
			while (c != eof && c != '=' && c != ':' && c != '\r' && c != '\n') { key += (char) c; c = istr.get(); }
			std::string value;
			if (c == '=' || c == ':')
			{
				c = readChar(istr);
				while (c != eof && c) { value += (char) c; c = readChar(istr); }
			}
			setRaw(trim(key), trim(value));
		}
	}
}

int TranslationPropertyFile::readChar(std::istream& istr)
{
	for (;;)
	{
		int c = istr.get();
		if (c == '\\')
		{
			c = istr.get();
			switch (c)
			{
			case 't':
				return '\t';
			case 'r':
				return '\r';
			case 'n':
				return '\n';
			case 'f':
				return '\f';
			case '\r':
				if (istr.peek() == '\n')
					istr.get();
				continue;
			case '\n':
				continue;
			default:
				return c;
			}
		}
		else if (c == '\n' || c == '\r')
			return 0;
		else
			return c;
	}
}
