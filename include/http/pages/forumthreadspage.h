#ifndef _forumthreads_page_
#define _forumthreads_page_

#include "forumpage.h"

class ForumThreadsPage:public ForumPage
{
public:
	ForumThreadsPage(SQLite3DB::DB *db, const std::string &templatestr):ForumPage(db,templatestr,"forumthreads.htm")	{}

	IPageHandler *New()	{ return new ForumThreadsPage(m_db,m_template); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);
};

#endif	// _forumthreads_page_
