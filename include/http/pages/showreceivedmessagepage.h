#ifndef _showrcvmessagepage_
#define _showrcvmessagepage_

#include "../ipagehandler.h"

class ShowReceivedMessagePage:public IPageHandler
{
public:
	ShowReceivedMessagePage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"showreceivedmessage.htm") {}

	IPageHandler *New()	{ return new ShowReceivedMessagePage(m_db,m_template); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
};

#endif	// _showrcvmessage_
