#ifndef _peertrustpage_
#define _peertrustpage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class PeerTrustPage:public IPageHandler,public IDatabase
{
public:
	PeerTrustPage(const std::string &templatestr):IPageHandler(templatestr)		{}

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string GetClassString(const std::string &trustlevel);

};

#endif	// _peertrustpage_
