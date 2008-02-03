#ifndef _optionspage_
#define _optionspage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class OptionsPage:public IPageHandler,public IDatabase
{
public:
	OptionsPage(const std::string &templatestr):IPageHandler(templatestr)	{}
	
private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
};

#endif	// _optionspage_
