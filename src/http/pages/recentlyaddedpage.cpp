#include "../../../include/http/pages/recentlyaddedpage.h"
#include "../../../include/global.h"
#include "../../../include/stringfunctions.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timespan.h>

const std::string RecentlyAddedPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";
	Poco::DateTime date;
	int count=0;
	std::string countstr="0";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="delete" && ValidateFormPassword(queryvars))
	{
		std::vector<std::string> identityids;
		CreateArgArray(queryvars,"chkdel",identityids);

		SQLite3DB::Statement del=m_db->Prepare("DELETE FROM tblIdentity WHERE IdentityID=?;");

		for(std::vector<std::string>::iterator i=identityids.begin(); i!=identityids.end(); i++)
		{
			if((*i)!="")
			{
				del.Bind(0,(*i));
				del.Step();
				del.Reset();
			}
		}

	}

	content="<h2>"+m_trans->Get("web.page.recentlyadded.title")+"</h2>";

	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID, PublicKey, Name, DateAdded, AddedMethod FROM tblIdentity WHERE DateAdded>=? ORDER BY DateAdded DESC;");
	date-=Poco::Timespan(5,0,0,0,0);
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
	st.Step();

	content+="<form name=\"frmdel\" method=\"post\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"delete\">";
	content+="<table class=\"small90\">";
	content+="<tr><th>"+m_trans->Get("web.page.recentlyadded.name")+"</th><th>"+m_trans->Get("web.page.recentlyadded.dateadded")+"</th><th>"+m_trans->Get("web.page.recentlyadded.addedmethod")+"</th></tr>";

	while(st.RowReturned())
	{
		std::string identityidstr="";
		std::string publickey="";
		std::string name="";
		std::string dateadded="";
		std::string addedmethod="";

		st.ResultText(0,identityidstr);
		st.ResultText(1,publickey);
		st.ResultText(2,name);
		st.ResultText(3,dateadded);
		st.ResultText(4,addedmethod);

		StringFunctions::Convert(count,countstr);

		content+="<tr>";
		content+="<td title=\""+publickey+"\">";
		content+="<a href=\"peerdetails.htm?identityid="+identityidstr+"\">";
		content+=SanitizeOutput(CreateShortIdentityName(name,publickey));
		content+="</a>";
		content+="</td>";
		content+="<td>"+dateadded+"</td>";
		content+="<td>"+SanitizeOutput(addedmethod)+"</td>";
		content+="<td><input type=\"checkbox\" name=\"chkdel["+countstr+"]\" value=\""+identityidstr+"\"></td>";
		content+="</tr>";

		count++;

		st.Step();
	}
	content+="<tr><td colspan=\"4\"><center><input type=\"submit\" value=\""+m_trans->Get("web.page.recentlyadded.deleteselected")+"\"></center></td></tr>";
	content+="</table>";

	return content;
}
