#ifndef _showpmessagepage_
#define _showpmessagepage_

#include "../ipagehandler.h"

class ShowPendingMessagePage:public IPageHandler
{
public:
	ShowPendingMessagePage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"showpendingmessage.htm") {}

	IPageHandler *New()	{ return new ShowPendingMessagePage(m_db,m_template); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
};

#endif	// _showpmessage_
