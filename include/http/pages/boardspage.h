#ifndef _boardspage_
#define _boardspage_

#include "../ipagehandler.h"

class BoardsPage:public IPageHandler
{
public:
	BoardsPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"boards.htm")	{}
	
	IPageHandler *New()	{ return new BoardsPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	
	const std::string BuildQueryString(const long startrow, const std::string &boardsearch);

};

#endif	// _boardspage_
