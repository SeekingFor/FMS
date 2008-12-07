#include "../../../include/http/pages/announceidentitypage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/global.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

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

		rval+="<option value=\""+id+"\" title=\""+pubkey+"\""+(selected==id?" selected":"")+">"+SanitizeOutput(CreateShortIdentityName(name,pubkey))+"</option>";
		st.Step();
	}
	rval+="</select>";
	return rval;
}

const std::string AnnounceIdentityPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	Poco::DateTime date;
	std::string content;
	int shown=0;
	std::string countstr="";
	std::string uuid;
	std::string lastid="";
	std::string thisid="";
	std::string day="";
	std::string name="";
	std::string pubkey="";
	int requestindex=0;
	bool willshow=false;
	std::string localidentityidstr="";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="announce" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement insert=m_db->Prepare("INSERT INTO tblIdentityIntroductionInserts(LocalIdentityID,Day,UUID,Solution) VALUES(?,?,?,?);");
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
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"announce\">";
	content+="<table>";
	content+="<tr><td colspan=\"4\"><center>Select Identity : ";
	content+=CreateLocalIdentityDropDown("localidentityid",localidentityidstr);
	content+="</td></tr>";
	content+="<tr><td colspan=\"4\"><center>Type the answers of a few of the following puzzles.  You don't need to get them all correct, but remember that they are case sensitive.  Getting announced will take some time and you must assign trust to other identities to see yourself announced.  DO NOT continuously solve captchas.  Solve 30 at most, wait a day, and if your identity has not been announced, repeat until it is.</td></tr>";
	content+="<tr>";

	date-=Poco::Timespan(1,0,0,0,0);
	SQLite3DB::Statement st=m_db->Prepare("SELECT UUID,Day,tblIdentity.IdentityID,RequestIndex,tblIdentity.Name,tblIdentity.PublicKey FROM tblIntroductionPuzzleRequests INNER JOIN tblIdentity ON tblIntroductionPuzzleRequests.IdentityID=tblIdentity.IdentityID WHERE UUID NOT IN (SELECT UUID FROM tblIdentityIntroductionInserts WHERE UUID IS NOT NULL) AND UUID NOT IN (SELECT UUID FROM tblIntroductionPuzzleInserts WHERE UUID IS NOT NULL) AND Day>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND Found='true' ORDER BY tblIdentity.IdentityID, Day DESC, RequestIndex DESC;");
	st.Step();

	if(st.RowReturned()==false)
	{
		content+="<td colspan=\"4\"><center>You must wait for some puzzles to be downloaded.  Make sure you have assigned trust to some other identities' trust lists and check back later.</td>";
	}
	
	while(st.RowReturned() && shown<20)
	{
		st.ResultText(0,uuid);
		st.ResultText(1,day);
		st.ResultText(2,thisid);
		st.ResultInt(3,requestindex);
		st.ResultText(4,name);
		st.ResultText(5,pubkey);

		// if we are already inserting a solution for an identity - we shouldn't show any puzzles that are older than the one we are inserting
		// get the last index # we are inserting this day from this identity
		// if the index here is greater than the index in the st statement, we will skip this puzzle because we are already inserting a puzzle with a greater index
		willshow=true;
		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblIdentityIntroductionInserts INNER JOIN tblIntroductionPuzzleRequests ON tblIdentityIntroductionInserts.UUID=tblIntroductionPuzzleRequests.UUID WHERE tblIdentityIntroductionInserts.Day=? AND tblIdentityIntroductionInserts.UUID IN (SELECT UUID FROM tblIntroductionPuzzleRequests WHERE IdentityID=? AND Day=? AND UUID IS NOT NULL) GROUP BY tblIdentityIntroductionInserts.Day;");
		st2.Step();
		if(st2.RowReturned()==true)
		{
			int index=0;
			st2.ResultInt(0,index);
			if(index>=requestindex)
			{
				willshow=false;
			}
		}

		if(willshow && thisid!=lastid)
		{
			StringFunctions::Convert(shown,countstr);
			if(shown>0 && shown%4==0)
			{
				content+="</tr>\r\n<tr>";
			}
			content+="<td title=\"From "+SanitizeOutput(CreateShortIdentityName(name,pubkey))+"\">";
			content+="<img src=\"showcaptcha.htm?UUID="+uuid+"\"><br>";
			content+="<input type=\"hidden\" name=\"uuid["+countstr+"]\" value=\""+uuid+"\">";
			content+="<input type=\"hidden\" name=\"day["+countstr+"]\" value=\""+day+"\">";
			content+="<input type=\"text\" name=\"solution["+countstr+"]\">";
			content+="</td>\r\n";
			lastid=thisid;
			shown++;
		}
		
		st.Step();
	}

	content+="</tr><td colspan=\"4\"><center><input type=\"submit\" value=\"Announce\"></td></tr>";
	content+="</table>";
	content+="</form>";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
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
