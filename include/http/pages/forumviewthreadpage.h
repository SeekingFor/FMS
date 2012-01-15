#ifndef _forumviewthreadpage_
#define _forumviewthreadpage_

#include "forumpage.h"
#include "../../../include/quoter.h"
#include "../../../include/http/emoticonreplacer.h"

class ForumTemplateViewThreadPage:public ForumTemplatePage
{
public:
	ForumTemplateViewThreadPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler);

	IPageHandler *New()	{ return new ForumTemplateViewThreadPage(m_db,m_templatehandler); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	const std::string GetPageTitle(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	const std::string FixBody(const std::string &body);
	const std::string FixUUIDAnchor(const std::string &uuid);
	
	bool m_showsmilies;
	bool m_detectlinks;
	int m_minlocalmessagetrust;
	int m_minpeermessagetrust;
	bool m_localtrustoverrides;
	QuoterHTMLRenderer m_htmlrenderer;
	std::string m_pagetitle;
	EmoticonReplacer m_emot;

};

#endif	// _forumviewthreadpage_
