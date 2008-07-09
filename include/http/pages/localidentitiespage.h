#ifndef _localidentitiespage_
#define _localidentitiespage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class LocalIdentitiesPage:public IPageHandler,public IDatabase
{
public:
	LocalIdentitiesPage(const std::string &templatestr):IPageHandler(templatestr)	{}

	IPageHandler *New()	{ return new LocalIdentitiesPage(m_template); }

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	void HandleUpdate(const std::map<std::string,std::string> &queryvars);
	void HandleDelete(const std::map<std::string,std::string> &queryvars);
	void HandleImport(const std::map<std::string,std::string> &queryvars);
	const std::string HandleExport();

};

#endif	// _localidentitiespage_
