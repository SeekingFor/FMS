#ifndef _forumcreatepostpage_
#define _forumcreatepostpage_

#include "forumpage.h"
/*
class ForumCreatePostPage:public ForumPage
{
public:
	ForumCreatePostPage(SQLite3DB::DB *db, const std::string &templatestr):ForumPage(db,templatestr,"forumcreatepost.htm")	{}

	IPageHandler *New()		{ return new ForumCreatePostPage(m_db,m_template); }
private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string LocalIdentityDropDown(const std::string &name, const std::string &selectedid);
	
};
*/
class ForumTemplateCreatePostPage:public ForumTemplatePage
{
public:
	ForumTemplateCreatePostPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forumcreatepost.htm")	{}

	IPageHandler *New()		{ return new ForumTemplateCreatePostPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);

};

#endif	// _forumcreatepostpage_
