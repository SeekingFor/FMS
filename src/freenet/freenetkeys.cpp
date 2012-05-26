#include "../../include/freenet/freenetkeys.h"

#include <cstdlib>
#include <sstream>
#include <Poco/RegularExpression.h>

/*
	KSK methods
*/

FreenetKSKKey::FreenetKSKKey():FreenetKey()
{

}

FreenetKSKKey::FreenetKSKKey(const std::string &key)
{
	if(TryParse(key)==false)
	{
		throw InvalidFreenetKeyException("Invalid KSK Key : "+key);
	}
}

const bool FreenetKSKKey::TryParse(const std::string &key)
{
	Poco::RegularExpression regex("^(freenet:)?KSK@.+",0);
	if(regex.match(key)==false)
	{
		m_fullkey="";
		return false;
	}
	
	m_fullkey=key;

	return true;
}


/*
	USK methods
*/
FreenetUSKKey::FreenetUSKKey():FreenetKey(),m_edition(0)
{

}

FreenetUSKKey::FreenetUSKKey(const std::string &key):m_edition(0)
{
	if(TryParse(key)==false)
	{
		throw InvalidFreenetKeyException("Invalid USK Key : "+key);
	}
}

const std::string FreenetUSKKey::GetBaseKey() const
{
	std::string basekey="";
	Poco::RegularExpression regex("^(freenet:)?(USK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/).+/-{0,1}[0-9]+(/.*)?",0);
	std::vector<std::string> matches;
	regex.split(m_fullkey,matches);

	if(matches.size()>2)
	{
		basekey=matches[2];
	}
	
	return basekey;
}

const std::string FreenetUSKKey::GetDataPath() const
{
	std::string datapath="";
	Poco::RegularExpression regex("^(freenet:)?USK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/.+/-{0,1}[0-9]+/(.*)",0);
	std::vector<std::string> matches;
	regex.split(m_fullkey,matches);

	if(matches.size()>2)
	{
		datapath=matches[2];
	}

	return datapath;
}

const std::string FreenetUSKKey::GetSiteName() const
{
	std::string sitename="";
	Poco::RegularExpression regex("^(freenet:)?USK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/(.+)/-{0,1}[0-9]+(/.*)?",0);
	std::vector<std::string> matches;
	regex.split(m_fullkey,matches);

	if(matches.size()>2)
	{
		sitename=matches[2];
	}

	return sitename;
}

const bool FreenetUSKKey::TryParse(const std::string &key)
{
	// USKs are USK@asdfasdf,asdfasdf,asdf/site/edition#/whatever - /whatever is optional
	Poco::RegularExpression regex("^(freenet:)?USK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/.+/-{0,1}[0-9]+(/.*)?",0);
	if(regex.match(key)==false)
	{
		m_edition=0;
		return false;
	}

	Poco::RegularExpression regex2("^(freenet:)?USK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/.+/(-{0,1}[0-9]+)(/.*)?",0);
	std::vector<std::string> matches;
	regex2.split(key,matches);
	if(matches.size()>2)
	{
		std::istringstream istr(matches[2]);
		if(!(istr >> m_edition))
		{
			m_edition=0;
		}
	}

	m_fullkey=key;
	return true;

}



/*
	SSK methods
*/

FreenetSSKKey::FreenetSSKKey():FreenetKey()
{

}

FreenetSSKKey::FreenetSSKKey(const std::string &key)
{
	if(TryParse(key)==false)
	{
		throw InvalidFreenetKeyException("Invalid SSK Key : "+key);
	}
}

const std::string FreenetSSKKey::GetBaseKey() const
{
	std::string basekey="";
	Poco::RegularExpression regex("^(freenet:)?(SSK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/).*",0);
	std::vector<std::string> matches;
	regex.split(m_fullkey,matches);

	if(matches.size()>2)
	{
		basekey=matches[2];
	}
	
	return basekey;
}

const std::string FreenetSSKKey::GetDataPath() const
{
	std::string datapath="";
	Poco::RegularExpression regex("^(freenet:)?SSK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/.+-{0,1}[0-9]+/(.*)",0);
	std::vector<std::string> matches;
	regex.split(m_fullkey,matches);

	if(matches.size()>2)
	{
		datapath=matches[2];
	}

	return datapath;
}

FreenetSSKKey &FreenetSSKKey::operator=(const FreenetUSKKey &rhs)
{
	std::ostringstream editionstr;
	editionstr << abs(rhs.GetEdition());
	TryParse("SSK@"+rhs.GetBaseKey().substr(4)+rhs.GetSiteName()+"-"+editionstr.str()+"/"+rhs.GetDataPath());
	return *this;
}

const bool FreenetSSKKey::TryParse(const std::string &key)
{
	Poco::RegularExpression regex("^(freenet:)?SSK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+(/.*)?");
	if(regex.match(key)==false)
	{
		m_fullkey="";
		return false;
	}

	m_fullkey=key;
	return true;

}


/*
	CHK methods
*/
FreenetCHKKey::FreenetCHKKey():FreenetKey()
{

}

FreenetCHKKey::FreenetCHKKey(const std::string &key)
{
	if(TryParse(key)==false)
	{
		throw InvalidFreenetKeyException("Invalid CHK Key : "+key);
	}
}

const bool FreenetCHKKey::TryParse(const std::string &key)
{
	// CHKs can technically just be the key without any filename
	Poco::RegularExpression regex("^(freenet:)?CHK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/{0,1}.*");
	if(regex.match(key)==false)
	{
		m_fullkey="";
		return false;
	}

	m_fullkey=key;
	return true;

}

const std::string FreenetCHKKey::GetBaseKey() const
{
	std::string basekey="";
	Poco::RegularExpression regex("^(freenet:)?(CHK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/{0,1}).*",0);
	std::vector<std::string> matches;
	regex.split(m_fullkey,matches);

	if(matches.size()>2)
	{
		basekey=matches[2];
	}
	
	return basekey;
}

const std::string FreenetCHKKey::GetDataPath() const
{
	std::string datapath="";
	Poco::RegularExpression regex("^(freenet:)?CHK@[A-Za-z0-9~-]+,[A-Za-z0-9~-]+,[A-Za-z0-9~-]+/{0,1}(.*)",0);
	std::vector<std::string> matches;
	regex.split(m_fullkey,matches);

	if(matches.size()>2)
	{
		datapath=matches[2];
	}

	return datapath;
}
