#ifndef _insertedfilespage_
#define _insertedfilespage_

#include "../ipagehandler.h"

class InsertedFilesPage:public IPageHandler
{
public:
	InsertedFilesPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"insertedfiles.htm")		{}

	IPageHandler *New()	{ return new InsertedFilesPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);

};

#endif	// _insertedfilespage_
