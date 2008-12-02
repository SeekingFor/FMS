#ifndef _optionspage_
#define _optionspage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class OptionsPage:public IPageHandler,public IDatabase
{
public:
	OptionsPage(const std::string &templatestr):IPageHandler(templatestr,"options.htm")	{}
	
	IPageHandler *New()	{ return new OptionsPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
	const std::string CreateDropDown(const std::string &name, const std::vector<std::string> &items, const std::string &selecteditem);
};

#endif	// _optionspage_
