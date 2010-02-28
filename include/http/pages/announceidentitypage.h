#ifndef _announceidentitypage_
#define _announceidentitypage_

#include "../ipagehandler.h"

class AnnounceIdentityPage:public IPageHandler
{
public:
	AnnounceIdentityPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"announceidentity.htm")	{}

	IPageHandler *New()	{ return new AnnounceIdentityPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);

	const std::string CreateLocalIdentityDropDown(const std::string &name, const std::string &selected);
};

#endif	// _announceidentitypage_
