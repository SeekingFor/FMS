#ifndef _browsemessagespage_
#define _browsemessagespage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class BrowseMessagesPage:public IPageHandler,public IDatabase
{
public:
	BrowseMessagesPage(const std::string &templatestr):IPageHandler(templatestr,"browsemessages.htm")	{}
	
	IPageHandler *New()	{ return new BrowseMessagesPage(m_template); }
	
private:
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
	const std::string BuildQueryString(const long startrow, const std::string &boardidstr, const std::string &messageidstr);
};

#endif	// _browsemessagespage_
