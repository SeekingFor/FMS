#ifndef _localidentitiespage_
#define _localidentitiespage_

#include "../ipagehandler.h"

class LocalIdentitiesPage:public IPageHandler
{
public:
	LocalIdentitiesPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"localidentities.htm")	{}

	IPageHandler *New()	{ return new LocalIdentitiesPage(m_db,m_template); }

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars);

	void HandleUpdate(const std::map<std::string,std::string> &queryvars);
	void HandleDelete(const std::map<std::string,std::string> &queryvars);
	void HandleImport(const std::map<std::string,std::string> &queryvars);
	const std::string HandleExport();

};

#endif	// _localidentitiespage_
