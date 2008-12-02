#ifndef _forumcreatepostpage_
#define _forumcreatepostpage_

#include "forumpage.h"

class ForumCreatePostPage:public ForumPage
{
public:
	ForumCreatePostPage(const std::string &templatestr):ForumPage(templatestr,"forumcreatepost.htm")	{}

	IPageHandler *New()		{ return new ForumCreatePostPage(m_template); }
private:
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string LocalIdentityDropDown(const std::string &name, const std::string &selectedid);
	
};

#endif	// _forumcreatepostpage_
