#ifndef _createidentitypage_
#define _createidentitypage_

#include "../ipagehandler.h"

class CreateIdentityPage:public IPageHandler
{
public:
	CreateIdentityPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"createidentity.htm")	{}

	IPageHandler *New()	{ return new CreateIdentityPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
};

#endif	// _createidentitypage_
