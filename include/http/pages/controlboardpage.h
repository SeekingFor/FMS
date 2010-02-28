#ifndef _controlboardpage_
#define _controlboardpage_

#include "../ipagehandler.h"

class ControlBoardPage:public IPageHandler
{
public:
	ControlBoardPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"controlboard.htm")	{}

	IPageHandler *New()	{ return new ControlBoardPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
};

#endif	// _controlboardpage_
