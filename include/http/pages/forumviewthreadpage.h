#ifndef _forumviewthreadpage_
#define _forumviewthreadpage_

#include "forumpage.h"
/*
class ForumViewThreadPage:public ForumPage
{
public:
	ForumViewThreadPage(SQLite3DB::DB *db,const std::string &templatestr);

	IPageHandler *New()		{ return new ForumViewThreadPage(m_db,m_template); }
private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string FixBody(const std::string &body);

	std::string m_fcphost;
	std::string m_fproxyport;

};
*/
class ForumTemplateViewThreadPage:public ForumTemplatePage
{
public:
	ForumTemplateViewThreadPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forumviewthread.htm")	{}

	IPageHandler *New()	{ return new ForumTemplateViewThreadPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string FixBody(const std::string &body);

};

#endif	// _forumviewthreadpage_
