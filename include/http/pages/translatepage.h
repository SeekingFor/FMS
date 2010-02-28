#ifndef _translatepage_
#define _translatepage_

#include "../ipagehandler.h"

class TranslatePage:public IPageHandler
{
public:
	TranslatePage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"translate.htm") {}

	IPageHandler *New()	{ return new TranslatePage(m_db,m_template); }

private:
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
};

#endif	// _translatepage_
