#ifndef _homepage_
#define _homepage_

#include "../ipagehandler.h"

class HomePage:public IPageHandler
{
public:
	HomePage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"index.htm") {}

	IPageHandler *New()	{ return new HomePage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
};

#endif	// _homepage_
