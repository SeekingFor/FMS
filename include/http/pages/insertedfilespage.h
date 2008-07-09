#ifndef _insertedfilespage_
#define _insertedfilespage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class InsertedFilesPage:public IPageHandler,public IDatabase
{
public:
	InsertedFilesPage(const std::string &templatestr):IPageHandler(templatestr)		{}

	IPageHandler *New()	{ return new InsertedFilesPage(m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

};

#endif	// _insertedfilespage_
