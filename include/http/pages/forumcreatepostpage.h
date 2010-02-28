#ifndef _forumcreatepostpage_
#define _forumcreatepostpage_

#include "forumpage.h"

class ForumTemplateCreatePostPage:public ForumTemplatePage
{
public:
	ForumTemplateCreatePostPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forumcreatepost.htm")	{}

	IPageHandler *New()		{ return new ForumTemplateCreatePostPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);

	struct fileattachment
	{
	public:
		fileattachment():m_id(0),m_filename(""),m_data(""),m_datasize(-1),m_contenttype(""),m_freenetkey("")	{}
		int m_id;
		std::string m_filename;
		std::string m_data;
		int m_datasize;
		std::string m_contenttype;
		std::string m_freenetkey;
	};

	void ClearFileAttachments(const std::string &viewstateid);
	void LoadFileAttachments(const std::string &viewstateid, std::vector<fileattachment> &fileattachments);

};

#endif	// _forumcreatepostpage_
