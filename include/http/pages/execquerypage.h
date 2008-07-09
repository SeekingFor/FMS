#ifndef _execquerypage_
#define _execquerypage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class ExecQueryPage:public IPageHandler,public IDatabase
{
public:
	ExecQueryPage(const std::string &templatestr):IPageHandler(templatestr)		{}

	IPageHandler *New()	{ return new ExecQueryPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
	
};

#endif	// _execquerypage_
