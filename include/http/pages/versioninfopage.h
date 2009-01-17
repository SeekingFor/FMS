#ifndef _versioninfopage_
#define _versioninfopage_

#include "../ipagehandler.h"

class VersionInfoPage:public IPageHandler
{
public:
	VersionInfoPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"versioninfo.htm")		{}

	IPageHandler *New()	{ return new VersionInfoPage(m_db,m_template); }

private:
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

};

#endif	// _versioninfopage_
