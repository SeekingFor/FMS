#ifndef _showimagepage_
#define _showimagepage_

#include "../ipagehandler.h"

class ShowImagePage:public IPageHandler
{
public:

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

	IPageHandler *New()	{ return new ShowImagePage; }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars) {return "";}

	static std::map<std::string,std::vector<char> > m_imagecache;

};

#endif	// _showcaptchapage_
