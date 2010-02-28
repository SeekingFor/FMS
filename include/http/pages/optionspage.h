#ifndef _optionspage_
#define _optionspage_

#include "../ipagehandler.h"

class OptionsPage:public IPageHandler
{
public:
	OptionsPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"options.htm")	{}
	
	IPageHandler *New()	{ return new OptionsPage(m_db,m_template); }

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);
	
	const std::string CreateTextBox(const std::string &name, const std::string &currentvalue, const std::string &param1, const std::string &param2);
	const std::string CreateDropDown(const std::string &option, const std::string &name, const std::vector<std::string> &items, const std::string &selecteditem, const std::string &param1, const std::string &param2);
	const std::string CreateTextArea(const std::string &name, const std::string &currentvalue, const std::string &param1, const std::string &param2);


	static int m_mode;

};

#endif	// _optionspage_
