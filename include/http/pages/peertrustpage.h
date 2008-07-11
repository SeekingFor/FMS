#ifndef _peertrustpage_
#define _peertrustpage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class PeerTrustPage:public IPageHandler,public IDatabase
{
public:
	PeerTrustPage(const std::string &templatestr):IPageHandler(templatestr)		{}

	IPageHandler *New()	{ return new PeerTrustPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string GetClassString(const std::string &trustlevel);
	const std::string BuildQueryString(const long startrow, const std::string &namesearch, const std::string &sortby, const std::string &sortorder, const int localidentityid);
	const std::string ReverseSort(const std::string &sortname, const std::string &currentsortby, const std::string &currentsortorder);
	const std::string CreateLocalIdentityDropDown(const std::string &name, const int selectedlocalidentityid);

};

#endif	// _peertrustpage_