#ifndef _peermaintenancepage_
#define _peermaintenancepage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class PeerMaintenancePage:public IPageHandler,public IDatabase
{
public:
	PeerMaintenancePage(const std::string &templatestr):IPageHandler(templatestr)	{}

	IPageHandler *New()	{ return new PeerMaintenancePage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

};

#endif	// _peermaintenancepage_
