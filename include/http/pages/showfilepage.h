#ifndef _showfilepage_
#define _showfilepage_

#include "../ipagehandler.h"

#include <map>
#include <set>

class ShowFilePage:public IPageHandler
{
public:
	ShowFilePage(SQLite3DB::DB *db);

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

	IPageHandler *New()	{ return new ShowFilePage(m_db); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars) {return "";}

	static std::map<std::string,std::string> m_filewhitelist;		// filename / content type

};

#endif	// _showfilepage_
