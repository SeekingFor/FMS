#ifndef _peerdetailspage_
#define _peerdetailspage_

#include "../ipagehandler.h"

class PeerDetailsPage:public IPageHandler
{
public:
	PeerDetailsPage(SQLite3DB::DB *db, const std::string templatestr):IPageHandler(db,templatestr,"peerdetails.htm")	{}

	IPageHandler *New()	{ return new PeerDetailsPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);

	const std::string GetClassString(const std::string &trustlevel);

};

#endif	// _peerdetailspage_
