#ifndef _boardspage_
#define _boardspage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class BoardsPage:public IPageHandler,public IDatabase
{
public:
	BoardsPage(const std::string &templatestr):IPageHandler(templatestr)	{}
	
	IPageHandler *New()	{ return new BoardsPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
	
	const std::string BuildQueryString(const long startrow, const std::string &boardsearch);

};

#endif	// _boardspage_
