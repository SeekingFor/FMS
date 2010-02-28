#ifndef _forummainpage_
#define _forummainpage_

#include "forumpage.h"

class ForumTemplateMainPage:public ForumTemplatePage
{
public:
	ForumTemplateMainPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forummain.htm")	{}

	IPageHandler *New()	{ return new ForumTemplateMainPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
};

#endif	// _forummainpage_
