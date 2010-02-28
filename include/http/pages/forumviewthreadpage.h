#ifndef _forumviewthreadpage_
#define _forumviewthreadpage_

#include "forumpage.h"
#include "../../../include/http/emoticonreplacer.h"

class ForumTemplateViewThreadPage:public ForumTemplatePage
{
public:
	ForumTemplateViewThreadPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler);

	IPageHandler *New()	{ return new ForumTemplateViewThreadPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	const std::string FixBody(const std::string &body);
	
	bool m_showsmilies;
	bool m_detectlinks;
	EmoticonReplacer m_emot;

};

#endif	// _forumviewthreadpage_
