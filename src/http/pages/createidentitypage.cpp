#include "../../../include/http/pages/createidentitypage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string CreateIdentityPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="create")
	{
		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblLocalIdentity(Name,PublishTrustList) VALUES(?,'true');");
		std::string name="";

		if(queryvars.find("name")!=queryvars.end())
		{
			name=(*queryvars.find("name")).second;
		}

		st.Bind(0,name);
		st.Step();

		content+="<h2>Created Identity</h2>";
	}
	else
	{
		content+="<h2>Create Identity</h2>";
		content+="<form name=\"frmcreateidentity\" method=\"POST\">";
		content+="<input type=\"hidden\" name=\"formaction\" value=\"create\">";
		content+="Name : <input type=\"text\" name=\"name\">";
		content+=" <input type=\"submit\" value=\"Create\">";
		content+="</form>";
	}

	return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"+StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool CreateIdentityPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("createidentity.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
