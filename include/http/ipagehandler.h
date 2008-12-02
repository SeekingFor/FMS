#ifndef _ipagehandler_
#define _ipagehandler_

#include "../ilogger.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <cstdlib>
#include <string>
#include <map>
#include <vector>

class IPageHandler:public Poco::Net::HTTPRequestHandler,public ILogger
{
public:
	IPageHandler()	{}
	IPageHandler(const std::string &templatestr, const std::string &pagename):m_template(templatestr),m_pagename(pagename)	{  }
	virtual ~IPageHandler()	{}
	virtual const bool WillHandleURI(const std::string &uri);

	virtual IPageHandler *New()=0;	// returns a new instance of the object

	virtual void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
	virtual const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)=0;

protected:
	// converts from basename[#] query args into a vector where the vector pos is the index pos #
	void CreateArgArray(const std::map<std::string,std::string> &vars, const std::string &basename, std::vector<std::string> &args);
	const std::string CreateTrueFalseDropDown(const std::string &name, const std::string &selected);

	void CreateQueryVarMap(Poco::Net::HTTPServerRequest &request, std::map<std::string,std::string> &vars);

	const std::string CreateFormPassword();
	const bool ValidateFormPassword(const std::map<std::string,std::string> &vars);

	// replaces html elements with encoded characters (i.e. < becomes &lt;)
	const std::string SanitizeOutput(const std::string &input);

	std::string m_template;
	std::string m_pagename;

};

#endif	// _ipagehandler_
