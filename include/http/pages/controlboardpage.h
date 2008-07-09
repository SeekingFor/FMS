#ifndef _controlboardpage_
#define _controlboardpage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class ControlBoardPage:public IPageHandler,public IDatabase
{
public:
	ControlBoardPage(const std::string &templatestr):IPageHandler(templatestr)	{}

	IPageHandler *New()	{ return new ControlBoardPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
};

#endif	// _controlboardpage_
