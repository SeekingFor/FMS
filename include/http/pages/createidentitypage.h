#ifndef _createidentitypage_
#define _createidentitypage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class CreateIdentityPage:public IPageHandler,public IDatabase
{
public:
	CreateIdentityPage(const std::string &templatestr):IPageHandler(templatestr,"createidentity.htm")	{}

	IPageHandler *New()	{ return new CreateIdentityPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
};

#endif	// _createidentitypage_
