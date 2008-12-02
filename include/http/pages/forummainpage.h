#ifndef _forummainpage_
#define _forummainpage_

#include "forumpage.h"

class ForumMainPage:public ForumPage
{
public:
	ForumMainPage(const std::string &templatestr):ForumPage(templatestr,"forummain.htm")	{}

	IPageHandler *New()	{ return new ForumMainPage(m_template); }

private:
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
};

#endif	// _forummainpage_
