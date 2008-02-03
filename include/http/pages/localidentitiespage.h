#ifndef _localidentitiespage_
#define _localidentitiespage_

#include "../ipagehandler.h"
#include "../../idatabase.h"

class LocalIdentitiesPage:public IPageHandler,public IDatabase
{
public:
	LocalIdentitiesPage(const std::string &templatestr):IPageHandler(templatestr)	{}
private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars);

	const std::string CreateTrueFalseDropDown(const std::string &name, const std::string &selected);

};

#endif	// _localidentitiespage_
