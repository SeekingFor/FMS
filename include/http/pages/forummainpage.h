#ifndef _forummainpage_
#define _forummainpage_

#include "forumpage.h"
/*
class ForumMainPage:public ForumPage
{
public:
	ForumMainPage(SQLite3DB::DB *db,const std::string &templatestr):ForumPage(db,templatestr,"forummain.htm")	{}

	IPageHandler *New()	{ return new ForumMainPage(m_db,m_template); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);
};
*/
class ForumTemplateMainPage:public ForumTemplatePage
{
public:
	ForumTemplateMainPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forummain.htm")	{}

	IPageHandler *New()	{ return new ForumTemplateMainPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);
};

#endif	// _forummainpage_
