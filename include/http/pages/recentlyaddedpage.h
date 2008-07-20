#ifndef _recentlyaddedpage_
#define _recentlyaddedpage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class RecentlyAddedPage:public IPageHandler,public IDatabase
{
public:
	RecentlyAddedPage(const std::string &templatestr):IPageHandler(templatestr)		{}

	IPageHandler *New()	{ return new RecentlyAddedPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

};

#endif	// _recentlyaddedpage_
