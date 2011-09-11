#ifndef _showavatarpage_
#define _showavatarpage_

#include "../ipagehandler.h"
#include "../../idatabase.h"
#include "../../../include/freenet/captcha/easybmp/EasyBMP.h"

class ShowAvatarPage:public IPageHandler
{
public:
	ShowAvatarPage(SQLite3DB::DB *db):IPageHandler(db)				{}

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

	IPageHandler *New()	{ return new ShowAvatarPage(m_db); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars) {return "";}
	const RGBApixel Interpolate(const RGBApixel &first, const RGBApixel &second, const double val);

};

#endif	// _showavatarpage_
