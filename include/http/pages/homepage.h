#ifndef _homepage_
#define _homepage_

#include "../ipagehandler.h"

class HomePage:public IPageHandler
{
public:
	HomePage(const std::string &templatestr):IPageHandler(templatestr) {}

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
};

#endif	// _homepage_
