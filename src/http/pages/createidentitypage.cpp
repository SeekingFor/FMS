#include "../../../include/http/pages/createidentitypage.h"
#include "../../../include/stringfunctions.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string CreateIdentityPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="create" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblLocalIdentity(Name,PublishTrustList,DateCreated) VALUES(?,'false',?);");
		std::string name="";
		Poco::DateTime date;

		if(queryvars.find("name")!=queryvars.end())
		{
			name=(*queryvars.find("name")).second;
		}

		st.Bind(0,name);
		st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
		st.Step();

		// insert all identities not in trust list already
		m_db->Execute("INSERT INTO tblIdentityTrust(LocalIdentityID,IdentityID) SELECT LocalIdentityID,IdentityID FROM tblLocalIdentity,tblIdentity WHERE LocalIdentityID || '_' || IdentityID NOT IN (SELECT LocalIdentityID || '_' || IdentityID FROM tblIdentityTrust);");

		content+="<h2>"+m_trans->Get("web.page.createidentity.createdidentity")+"</h2>";
		content+=m_trans->Get("web.page.createidentity.aftercreateinstructions");
	}
	else
	{
		content+="<h2>"+m_trans->Get("web.page.createidentity.title")+"</h2>";
		content+="<form name=\"frmcreateidentity\" method=\"POST\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"formaction\" value=\"create\">";
		content+="Name : <input type=\"text\" name=\"name\" maxlength=\"40\">";
		content+=" <input type=\"submit\" value=\""+m_trans->Get("web.page.createidentity.create")+"\">";
		content+="</form>";
	}

	return content;
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
