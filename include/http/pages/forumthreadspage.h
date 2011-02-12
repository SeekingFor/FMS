#ifndef _forumthreads_page_
#define _forumthreads_page_

#include "forumpage.h"

class ForumTemplateThreadsPage:public ForumTemplatePage
{
public:
	ForumTemplateThreadsPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forumthreads.htm")	{ m_pagetitle=GetBasePageTitle(); }

	IPageHandler *New()	{ return new ForumTemplateThreadsPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	const std::string GetPageTitle(const std::string &method, const std::map<std::string,QueryVar> &queryvars)			{ return m_pagetitle; }

	const std::string CreateForumSearchBoxExtraFields() const;

	std::string m_pagetitle;
};

#endif	// _forumthreads_page_
