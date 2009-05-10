#ifndef _ipagehandler_
#define _ipagehandler_

#include "../ilogger.h"
#include "../idatabase.h"
#include "../global.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <cstdlib>
#include <string>
#include <map>
#include <vector>

class IPageHandler:public Poco::Net::HTTPRequestHandler,public ILogger,public IDatabase
{
public:
	IPageHandler(SQLite3DB::DB *db):IDatabase(db)		{ m_trans=Translation.get(); }
	IPageHandler(SQLite3DB::DB *db, const std::string &templatestr, const std::string &pagename):IDatabase(db),m_template(templatestr),m_pagename(pagename)	{ m_trans=Translation.get(); }
	virtual ~IPageHandler()	{}
	virtual const bool WillHandleURI(const std::string &uri);

	virtual IPageHandler *New()=0;	// returns a new instance of the object

	virtual void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
	virtual const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)=0;

protected:
	virtual const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	// converts from basename[#] query args into a vector where the vector pos is the index pos #
	void CreateArgArray(const std::map<std::string,std::string> &vars, const std::string &basename, std::vector<std::string> &args);
	const std::string CreateTrueFalseDropDown(const std::string &name, const std::string &selected);

	void CreateQueryVarMap(Poco::Net::HTTPServerRequest &request, std::map<std::string,std::string> &vars);

	const std::string CreateFormPassword();
	const bool ValidateFormPassword(const std::map<std::string,std::string> &vars);

	// replaces html elements with encoded characters (i.e. < becomes &lt;)
	const std::string SanitizeOutput(const std::string &input);
	// don't replace space with &nbsp;, because browser might convert to unicode non breaking space character
	const std::string SanitizeTextAreaOutput(const std::string &input);

	const std::string GenerateNavigationLinks();

	std::string m_template;
	std::string m_pagename;

	StringTranslation *m_trans;

};

#endif	// _ipagehandler_
