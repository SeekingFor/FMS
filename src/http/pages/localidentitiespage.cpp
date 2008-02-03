#include "../../../include/http/pages/localidentitiespage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string LocalIdentitiesPage::CreateTrueFalseDropDown(const std::string &name, const std::string &selected)
{
	std::string rval="";

	rval+="<select name=\""+name+"\">";
	rval+="<option value=\"true\"";
	if(selected=="true")
	{
		rval+=" SELECTED";
	}
	rval+=">true</option>";
	rval+="<option value=\"false\"";
	if(selected=="false")
	{
		rval+=" SELECTED";
	}
	rval+=">false</option>";
	rval+="</select>";

	return rval;
}

const std::string LocalIdentitiesPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	int count;
	std::string countstr;
	std::string content="";


	if(queryvars.find("formaction")!=queryvars.end())
	{
		int id;
		std::vector<std::string> ids;
		std::vector<std::string> singleuse;
		std::vector<std::string> publishtrustlist;

		CreateArgArray(queryvars,"chkidentityid",ids);
		CreateArgArray(queryvars,"singleuse",singleuse);
		CreateArgArray(queryvars,"publishtrustlist",publishtrustlist);

		if((*queryvars.find("formaction")).second=="update")
		{
			SQLite3DB::Statement update=m_db->Prepare("UPDATE tblLocalIdentity SET SingleUse=?, PublishTrustList=? WHERE LocalIdentityID=?;");
			for(int i=0; i<ids.size(); i++)
			{
				if(ids[i]!="")
				{
					StringFunctions::Convert(ids[i],id);
					update.Bind(0,singleuse[i]);
					update.Bind(1,publishtrustlist[i]);
					update.Bind(2,id);
					update.Step();
					update.Reset();
				}
			}
		}
		if((*queryvars.find("formaction")).second=="delete")
		{
			SQLite3DB::Statement del=m_db->Prepare("DELETE FROM tblLocalIdentity WHERE LocalIdentityID=?;");
			for(int i=0; i<ids.size(); i++)
			{
				if(ids[i]!="")
				{
					StringFunctions::Convert(ids[i],id);
					del.Bind(0,id);
					del.Step();
					del.Reset();
				}
			}
		}
	}

	content+="<h2>Local Identities</h2>";
	content+="<form name=\"frmlocalidentity\" method=\"POST\">";
	content+="<input type=\"hidden\" name=\"formaction\" value=\"update\">";
	content+="<table><tr><td></td><th>Name</th><th>Single Use</th><th>Publish Trust List</th><th>Announced? *</th></tr>";

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID,tblLocalIdentity.Name,tblLocalIdentity.PublicKey,tbLLocalIdentity.PublishTrustList,tblLocalIdentity.SingleUse,tblLocalIdentity.PublishBoardList,tblIdentity.IdentityID FROM tblLocalIdentity LEFT JOIN tblIdentity ON tblLocalIdentity.PublicKey=tblIdentity.PublicKey ORDER BY tblLocalIdentity.Name;");
	st.Step();

	count=0;
	while(st.RowReturned())
	{
		StringFunctions::Convert(count,countstr);
		std::string id;
		std::string name;
		std::string publickey;
		std::string publishtrustlist;
		std::string singleuse;

		st.ResultText(0,id);
		st.ResultText(1,name);
		st.ResultText(2,publickey);
		st.ResultText(3,publishtrustlist);
		st.ResultText(4,singleuse);

		content+="<tr>";
		content+="<td><input type=\"checkbox\" name=\"chkidentityid["+countstr+"]\" value=\""+id+"\"></td>";
		content+="<td title=\""+publickey+"\">"+name+"</td>";
		content+="<td>"+CreateTrueFalseDropDown("singleuse["+countstr+"]",singleuse)+"</td>";
		content+="<td>"+CreateTrueFalseDropDown("publishtrustlist["+countstr+"]",publishtrustlist)+"</td>";
		if(st.ResultNull(6))
		{
			content+="<td>No</td>";
		}
		else
		{
			content+="<td>Yes</td>";
		}
		content+="</tr>";
		st.Step();
		count++;
	}

	content+="<tr><td colspan=\"4\"><center><input type=\"submit\" value=\"Update Selected\"> <input type=\"submit\" value=\"Delete Selected\" onClick=\"if(confirm('Delete Selected Identities?')){frmlocalidentity.formaction.value='delete';}else{return false;}\"></td></tr>";
	content+="</table>";
	content+="<p class=\"paragraph=\">* An identity is considered successfully announced when you have downloaded a trust list from someone that contains the identity.</p>";

	return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"+StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool LocalIdentitiesPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("localidentities.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
