#ifndef _showimagepage_
#define _showimagepage_

#include "../ipagehandler.h"

#include <map>
#include <set>

class ShowImagePage:public IPageHandler
{
public:
	ShowImagePage(SQLite3DB::DB *db);

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

	IPageHandler *New()	{ return new ShowImagePage(m_db); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars) {return "";}

	static std::map<std::string,std::vector<char> > m_imagecache;
	static std::set<std::string> m_imagewhitelist;

};

#endif	// _showcaptchapage_
