#include "../../../include/http/pages/announceidentitypage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/datetime.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string AnnounceIdentityPage::CreateLocalIdentityDropDown(const std::string &name, const std::string &selected)
{
	std::string rval="";
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, Name, PublicKey FROM tblLocalIdentity ORDER BY Name;");
	st.Step();

	rval+="<select name=\""+name+"\">";
	while(st.RowReturned())
	{
		std::string id;
		std::string name;
		std::string pubkey;

		st.ResultText(0,id);
		st.ResultText(1,name);
		st.ResultText(2,pubkey);

		rval+="<option value=\""+id+"\" title=\""+pubkey+"\">"+name+"</option>";
		st.Step();
	}
	rval+="</select>";
	return rval;
}

const std::string AnnounceIdentityPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	DateTime date;
	std::string content;
	int shown=0;
	std::string countstr="";
	std::string uuid;
	std::string lastid="";
	std::string thisid="";
	std::string day="";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="announce")
	{
		SQLite3DB::Statement insert=m_db->Prepare("INSERT INTO tblIdentityIntroductionInserts(LocalIdentityID,Day,UUID,Solution) VALUES(?,?,?,?);");
		std::string localidentityidstr="";
		int localidentityid=0;
		std::vector<std::string> uuids;
		std::vector<std::string> days;
		std::vector<std::string> solutions;

		if(queryvars.find("localidentityid")!=queryvars.end())
		{
			localidentityidstr=(*queryvars.find("localidentityid")).second;
			StringFunctions::Convert(localidentityidstr,localidentityid);
		}
		CreateArgArray(queryvars,"uuid",uuids);
		CreateArgArray(queryvars,"day",days);
		CreateArgArray(queryvars,"solution",solutions);

		for(int i=0; i<solutions.size(); i++)
		{
			if(solutions[i]!="")
			{
				insert.Bind(0,localidentityid);
				insert.Bind(1,days[i]);
				insert.Bind(2,uuids[i]);
				insert.Bind(3,solutions[i]);
				insert.Step();
				insert.Reset();
			}
		}

	}

	content+="<h2>Announce Identity</h2>";
	content+="<form name=\"frmannounce\" method=\"POST\">";
	content+="<input type=\"hidden\" name=\"formaction\" value=\"announce\">";
	content+="<table>";
	content+="<tr><td colspan=\"4\"><center>Select Identity : ";
	content+=CreateLocalIdentityDropDown("localidentityid","");
	content+="</td></tr>";
	content+="<tr><td colspan=\"4\"><center>Type the answers of a few puzzles</td></tr>";
	content+="<tr>";

	date.SetToGMTime();
	date.Add(0,0,0,-1);
	SQLite3DB::Statement st=m_db->Prepare("SELECT UUID,Day,IdentityID FROM tblIntroductionPuzzleRequests WHERE UUID NOT IN (SELECT UUID FROM tblIdentityIntroductionInserts) AND UUID NOT IN (SELECT UUID FROM tblIntroductionPuzzleInserts) AND Day>='"+date.Format("%Y-%m-%d")+"' AND Found='true' ORDER BY IdentityID, Day DESC, RequestIndex DESC;");
	st.Step();

	if(st.RowReturned()==false)
	{
		content+="<td colspan=\"4\"><center>You must wait for some puzzles to be downloaded.  Check back later.</td>";
	}
	
	while(st.RowReturned() && shown<20)
	{
		st.ResultText(0,uuid);
		st.ResultText(1,day);
		st.ResultText(2,thisid);

		if(thisid!=lastid)
		{
			StringFunctions::Convert(shown,countstr);
			if(shown>0 && shown%4==0)
			{
				content+="</tr>\r\n<tr>";
			}
			content+="<td>";
			content+="<img src=\"showcaptcha.htm?UUID="+uuid+"\"><br>";
			content+="<input type=\"hidden\" name=\"uuid["+countstr+"]\" value=\""+uuid+"\">";
			content+="<input type=\"hidden\" name=\"day["+countstr+"]\" value=\""+day+"\">";
			content+="<input type=\"text\" name=\"solution["+countstr+"]\">";
			content+="</td>\r\n";
			thisid=lastid;
			shown++;
		}
		
		st.Step();
	}

	content+="</tr><td colspan=\"4\"><center><input type=\"submit\" value=\"Announce\"></td></tr>";
	content+="</table>";
	content+="</form>";

	return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"+StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool AnnounceIdentityPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("announceidentity.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
