#ifndef _confirmpage_
#define _confirmpage_

#include "../ipagehandler.h"

class ConfirmPage:public IPageHandler
{
public:
	ConfirmPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"confirm.htm")		{}

	IPageHandler *New()	{ return new ConfirmPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	
};

#endif	// _confirmpage_
