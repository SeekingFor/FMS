#ifndef _recentlyaddedpage_
#define _recentlyaddedpage_

#include "../ipagehandler.h"

class RecentlyAddedPage:public IPageHandler
{
public:
	RecentlyAddedPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"recentlyadded.htm")		{}

	IPageHandler *New()	{ return new RecentlyAddedPage(m_db,m_template); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);

};

#endif	// _recentlyaddedpage_
