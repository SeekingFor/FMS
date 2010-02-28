#ifndef _addpeerpage_
#define _addpeerpage_

#include "../ipagehandler.h"

class AddPeerPage:public IPageHandler
{
public:
	AddPeerPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"addpeer.htm")	{}

	IPageHandler *New()	{ return new AddPeerPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	
};

#endif	// _addpeerpage_
