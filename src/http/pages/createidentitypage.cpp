#include "../../../include/http/pages/createidentitypage.h"
#include "../../../include/stringfunctions.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string CreateIdentityPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
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

		content+="<h2>Created Identity</h2>";
		content+="You must have at least 1 local identity that has set explicit trust list trust for one or more peers who are publishing trust lists or you will not be able to learn about other identities.";
	}
	else
	{
		content+="<h2>Create Identity</h2>";
		content+="<form name=\"frmcreateidentity\" method=\"POST\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"formaction\" value=\"create\">";
		content+="Name : <input type=\"text\" name=\"name\" maxlength=\"40\">";
		content+=" <input type=\"submit\" value=\"Create\">";
		content+="</form>";
	}

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
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
