#ifndef _ipagehandler_
#define _ipagehandler_

#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <shttpd.h>

class IPageHandler
{
public:
	IPageHandler()	{}
	IPageHandler(const std::string &templatestr)	{ m_template=templatestr; }
	virtual ~IPageHandler()	{}

	/**
		\brief Handles request for a page
		
		\return true if request was handled, false if it was ignored
	*/
	const bool Handle(shttpd_arg *arg);

private:
	void HandlePost(shttpd_arg *arg);
	void HadleGet(shttpd_arg *arg);
	virtual const bool WillHandleURI(const std::string &uri)=0;
	virtual const std::string GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)=0;
	
protected:
	// converts from basename[#] query args into a vector where the vector pos is the index pos #
	void CreateArgArray(const std::map<std::string,std::string> &vars, const std::string &basename, std::vector<std::string> &args);

	std::string m_template;

};

#endif	// _ipagehandler_
