#ifndef _forumsearch_page_
#define _forumsearch_page_

#include "forumpage.h"

class ForumSearchPage:public ForumTemplatePage
{
public:
	ForumSearchPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forumsearch.htm")	{}

	IPageHandler *New()	{ return new ForumSearchPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);

	const std::string CreateForumDropDown(const std::string &name, const int selectedforumid) const;
	const std::string CreateSortByDropDown(const std::string &name, const std::string &selecteditem) const;
	const std::string CreateSortOrderDropDown(const std::string &name, const std::string &selecteditem) const;

	struct searchitem
	{
		std::string m_phrase;
		bool m_include;
		int m_group;
	};

	void SeparateSearchItems(const std::string &querystring, std::vector<searchitem> &searchitems);
	const std::string CreateSQLCriteriaClause(const std::vector<searchitem> &searchitems, const std::string &fieldname) const;

};

#endif	// _forumsearch_page_
