#ifndef _announceidentitypage_
#define _announceidentitypage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class AnnounceIdentityPage:public IPageHandler,public IDatabase
{
public:
	AnnounceIdentityPage(const std::string &templatestr):IPageHandler(templatestr)	{}

	IPageHandler *New()	{ return new AnnounceIdentityPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string CreateLocalIdentityDropDown(const std::string &name, const std::string &selected);
};

#endif	// _announceidentitypage_
