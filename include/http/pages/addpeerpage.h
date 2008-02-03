#ifndef _addpeerpage_
#define _addpeerpage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class AddPeerPage:public IPageHandler,public IDatabase
{
public:
	AddPeerPage(const std::string &templatestr):IPageHandler(templatestr)	{}

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);
	
};

#endif	// _addpeerpage_
