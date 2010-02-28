#ifndef _showcaptchapage_
#define _showcaptchapage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class ShowCaptchaPage:public IPageHandler
{
public:
	ShowCaptchaPage(SQLite3DB::DB *db):IPageHandler(db)				{}

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

	IPageHandler *New()	{ return new ShowCaptchaPage(m_db); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars) {return "";}

};

#endif	// _showcaptchapage_
