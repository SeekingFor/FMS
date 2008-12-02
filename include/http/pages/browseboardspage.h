#ifndef _browseboardspage_
#define _browseboardspage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class BrowseBoardsPage:public IPageHandler,public IDatabase
{
public:
	BrowseBoardsPage(const std::string &templatestr):IPageHandler(templatestr,"boardsbrowse.htm")	{}
	
	IPageHandler *New()	{ return new BrowseBoardsPage(m_template); }
	
private:
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string BuildQueryString(const long startrow, const std::string &boardsearch, const std::string &sortby, const std::string &sortorder);
	const std::string ReverseSort(const std::string &sortname, const std::string &currentsortby, const std::string &currentsortorder);
};

#endif	// _browseboardspage_
