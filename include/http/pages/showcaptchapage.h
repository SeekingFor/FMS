#ifndef _showcaptchapage_
#define _showcaptchapage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class ShowCaptchaPage:public IPageHandler,public IDatabase
{
public:

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

	IPageHandler *New()	{ return new ShowCaptchaPage; }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars) {return "";}

};

#endif	// _showcaptchapage_
