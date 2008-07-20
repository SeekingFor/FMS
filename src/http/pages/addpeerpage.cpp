#include "../../../include/http/pages/addpeerpage.h"
#include "../../../include/stringfunctions.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string AddPeerPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="add" && ValidateFormPassword(queryvars))
	{
		Poco::DateTime date;
		std::string publickey="";
		if(queryvars.find("publickey")!=queryvars.end())
		{
			publickey=(*queryvars.find("publickey")).second;
		}
		if(publickey!="" && publickey.find("SSK@")==0 && publickey[publickey.size()-1]=='/')
		{
			SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES(?,?,?);");
			st.Bind(0,publickey);
			st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
			st.Bind(2,"manually");
			st.Step();
			st.Reset();
		}
	}

	content+="<h2>Add Peer</h2>";
	content+="<form name=\"frmaddpeer\" method=\"POST\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"add\">";
	content+="Public Key : ";
	content+="<input type=\"text\" name=\"publickey\" size=\"100\">";
	content+="<br>";
	content+="The public key must be a valid SSK public key and include the / at the end";
	content+="<br>";
	content+="<input type=\"submit\" value=\"Add\">";
	content+="</form>";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool AddPeerPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("addpeer.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
