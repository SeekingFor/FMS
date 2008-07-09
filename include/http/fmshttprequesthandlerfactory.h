#ifndef _fmshttprequesthandlerfactory_
#define _fmshttprequesthandlerfactory_

#include "ipagehandler.h"
#include "../ipaddressacl.h"
#include "../ilogger.h"

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

class FMSHTTPRequestHandlerFactory:public Poco::Net::HTTPRequestHandlerFactory,public ILogger
{
public:
	FMSHTTPRequestHandlerFactory();
	~FMSHTTPRequestHandlerFactory();

	Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request);

private:
	std::vector<IPageHandler *> m_pagehandlers;
	IPAddressACL m_acl;

};

#endif	// _fmshttprequesthandlerfactory_
