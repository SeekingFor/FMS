#ifndef _confirmpage_
#define _confirmpage_

#include "../ipagehandler.h"

class ConfirmPage:public IPageHandler
{
public:
	ConfirmPage(const std::string &templatestr):IPageHandler(templatestr)		{}

	IPageHandler *New()	{ return new ConfirmPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
	
};

#endif	// _confirmpage_
