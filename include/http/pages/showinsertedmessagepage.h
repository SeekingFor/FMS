#ifndef _showimessagepage_
#define _showimessagepage_

#include "../ipagehandler.h"

class ShowInsertedMessagePage:public IPageHandler
{
public:
	ShowInsertedMessagePage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"showinsertedmessage.htm") {}

	IPageHandler *New()	{ return new ShowInsertedMessagePage(m_db,m_template); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
};

#endif	// _showimessage_
