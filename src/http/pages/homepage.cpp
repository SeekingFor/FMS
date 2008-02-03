#include "../../../include/http/pages/homepage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string HomePage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="<h2>Home</h2>";
	content+="<p class=\"paragraph\">";
	content+="Use these pages to administer your FMS installation.";
	content+="</p>";
	return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"+StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool HomePage::WillHandleURI(const std::string &uri)
{
	return true;
}
