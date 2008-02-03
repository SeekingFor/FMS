#ifndef _showcaptchapage_
#define _showcaptchapage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class ShowCaptchaPage:public IPageHandler,public IDatabase
{
public:

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

};

#endif	// _showcaptchapage_
