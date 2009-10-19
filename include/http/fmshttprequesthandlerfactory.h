#ifndef _fmshttprequesthandlerfactory_
#define _fmshttprequesthandlerfactory_

#include "ipagehandler.h"
#include "htmltemplatehandler.h"
#include "../ipaddressacl.h"
#include "../ilogger.h"
#include "../idatabase.h"

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

class FMSHTTPRequestHandlerFactory:public Poco::Net::HTTPRequestHandlerFactory,public ILogger,public IDatabase
{
public:
	FMSHTTPRequestHandlerFactory(SQLite3DB::DB *db);
	~FMSHTTPRequestHandlerFactory();

	Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request);

private:
	std::vector<IPageHandler *> m_pagehandlers;
	IPAddressACL m_acl;
	HTMLTemplateHandler m_forumtemplatehandler;

};

#endif	// _fmshttprequesthandlerfactory_
