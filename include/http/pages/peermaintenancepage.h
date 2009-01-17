#ifndef _peermaintenancepage_
#define _peermaintenancepage_

#include "../ipagehandler.h"

class PeerMaintenancePage:public IPageHandler
{
public:
	PeerMaintenancePage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"peermaintenance.htm")	{}

	IPageHandler *New()	{ return new PeerMaintenancePage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

};

#endif	// _peermaintenancepage_
