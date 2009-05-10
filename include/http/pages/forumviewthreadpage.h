#ifndef _forumviewthreadpage_
#define _forumviewthreadpage_

#include "forumpage.h"

class ForumViewThreadPage:public ForumPage
{
public:
	ForumViewThreadPage(SQLite3DB::DB *db,const std::string &templatestr):ForumPage(db,templatestr,"forumviewthread.htm")	{}

	IPageHandler *New()		{ return new ForumViewThreadPage(m_db,m_template); }
private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string FixBody(const std::string &body);

};

#endif	// _forumviewthreadpage_
