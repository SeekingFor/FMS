#ifndef _homepage_
#define _homepage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class HomePage:public IPageHandler,public IDatabase
{
public:
	HomePage(const std::string &templatestr):IPageHandler(templatestr,"index.htm") {}

	IPageHandler *New()	{ return new HomePage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
};

#endif	// _homepage_
