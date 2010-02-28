#ifndef _execquerypage_
#define _execquerypage_

#include "../ipagehandler.h"

class ExecQueryPage:public IPageHandler
{
public:
	ExecQueryPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"execquery.htm")		{}

	IPageHandler *New()	{ return new ExecQueryPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	
};

#endif	// _execquerypage_
