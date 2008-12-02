#ifndef _versioninfopage_
#define _versioninfopage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class VersionInfoPage:public IPageHandler,public IDatabase
{
public:
	VersionInfoPage(const std::string &templatestr):IPageHandler(templatestr,"versioninfo.htm")		{}

	IPageHandler *New()	{ return new VersionInfoPage(m_template); }

private:
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

};

#endif	// _versioninfopage_
