#ifndef _peerdetailspage_
#define _peerdetailspage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class PeerDetailsPage:public IPageHandler,public IDatabase
{
public:
	PeerDetailsPage(const std::string templatestr):IPageHandler(templatestr)	{}

	IPageHandler *New()	{ return new PeerDetailsPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string GetClassString(const std::string &trustlevel);

};

#endif	// _peerdetailspage_
