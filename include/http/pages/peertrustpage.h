#ifndef _peertrustpage_
#define _peertrustpage_

#include "../ipagehandler.h"

class PeerTrustPage:public IPageHandler
{
public:
	PeerTrustPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"peertrust.htm")		{}

	IPageHandler *New()	{ return new PeerTrustPage(m_db,m_template); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);

	const std::string GetClassString(const std::string &trustlevel);
	const std::string BuildQueryString(const long startrow, const std::string &namesearch, const std::string &sortby, const std::string &sortorder, const int localidentityid);
	const std::string ReverseSort(const std::string &sortname, const std::string &currentsortby, const std::string &currentsortorder);
	const std::string CreateLocalIdentityDropDown(const std::string &name, const int selectedlocalidentityid);

};

#endif	// _peertrustpage_
