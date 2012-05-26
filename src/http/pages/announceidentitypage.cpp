#include "../../../include/http/pages/announceidentitypage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/global.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#include <algorithm>

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

const std::string AnnounceIdentityPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
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
	std::string captchatype="";
	int requestindex=0;
	bool willshow=false;
	std::string localidentityidstr="";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="announce" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement insert=m_db->Prepare("INSERT INTO tblIdentityIntroductionInserts(LocalIdentityID,Day,UUID,Solution) VALUES(?,?,?,?);");
		SQLite3DB::Statement inserttrustst=m_db->Prepare("INSERT OR IGNORE INTO tblIdentityTrust(LocalIdentityID,IdentityID) VALUES(?,?);");
		SQLite3DB::Statement addtrustst=m_db->Prepare("UPDATE tblIdentityTrust SET LocalTrustListTrust=? WHERE LocalIdentityID=? AND IdentityID=? AND LocalTrustListTrust IS NULL;");
		int localidentityid=0;
		std::vector<std::string> uuids;
		std::vector<std::string> days;
		std::vector<std::string> solutions;
		std::vector<std::string> captchatypes;
		std::vector<std::string> unlikenums;
		std::vector<std::string> unlikeobjects;
		std::vector<std::string> similarobjects;
		std::vector<std::string> identityids;
		bool addtrust=false;
		int trust=0;

		if(queryvars.find("chkaddtrust")!=queryvars.end() && (*queryvars.find("chkaddtrust")).second!="" && queryvars.find("txtaddtrust")!=queryvars.end() && (*queryvars.find("txtaddtrust")).second!="")
		{
			if(StringFunctions::Convert((*queryvars.find("txtaddtrust")).second.GetData(),trust))
			{
				trust=(std::max)(0,(std::min)(100,trust));
				addtrust=true;
			}
		}

		if(queryvars.find("localidentityid")!=queryvars.end())
		{
			localidentityidstr=(*queryvars.find("localidentityid")).second.GetData();
			StringFunctions::Convert(localidentityidstr,localidentityid);
		}
		CreateArgArray(queryvars,"uuid",uuids);
		CreateArgArray(queryvars,"day",days);
		CreateArgArray(queryvars,"solution",solutions);
		CreateArgArray(queryvars,"captchatype",captchatypes);
		CreateArgArray(queryvars,"unlikenum",unlikenums);
		CreateArgArray(queryvars,"unlikeobject",unlikeobjects);
		CreateArgArray(queryvars,"similarobject",similarobjects);
		CreateArgArray(queryvars,"identityid",identityids);

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

				if(addtrust==true)
				{
					inserttrustst.Bind(0,localidentityid);
					inserttrustst.Bind(1,identityids[i]);
					inserttrustst.Step();
					inserttrustst.Reset();

					addtrustst.Bind(0,trust);
					addtrustst.Bind(1,localidentityid);
					addtrustst.Bind(2,identityids[i]);
					addtrustst.Step();
					addtrustst.Reset();
				}
			}
		}

		for(int i=0; i<unlikenums.size(); i++)
		{
			if(unlikeobjects[i]!="" && similarobjects[i]!="")
			{
				insert.Bind(0,localidentityid);
				insert.Bind(1,days[i]);
				insert.Bind(2,uuids[i]);
				StringFunctions::LowerCase(unlikeobjects[i],unlikeobjects[i]);
				StringFunctions::LowerCase(similarobjects[i],similarobjects[i]);
				insert.Bind(3,std::string(unlikenums[i]+unlikeobjects[i]+similarobjects[i]));
				insert.Step();
				insert.Reset();
			}
		}

	}

	content+="<h2>"+m_trans->Get("web.page.announceidentity.title")+"</h2>";
	content+="<form name=\"frmannounce\" method=\"POST\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"announce\">";
	content+="<table>";
	content+="<tr><td colspan=\"4\"><center>"+m_trans->Get("web.page.announceidentity.selectidentity")+" ";
	content+=CreateLocalIdentityDropDown("localidentityid",localidentityidstr);
	content+="</td></tr>";
	content+="<tr><td colspan=\"4\"><center>"+m_trans->Get("web.page.announceidentity.instructions")+"</td></tr>";
	content+="<tr>";

	date-=Poco::Timespan(1,0,0,0,0);
	SQLite3DB::Statement st=m_db->Prepare("SELECT UUID,Day,tblIdentity.IdentityID,RequestIndex,tblIdentity.Name,tblIdentity.PublicKey,tblIntroductionPuzzleRequests.Type FROM tblIntroductionPuzzleRequests INNER JOIN tblIdentity ON tblIntroductionPuzzleRequests.IdentityID=tblIdentity.IdentityID WHERE UUID NOT IN (SELECT UUID FROM tblIdentityIntroductionInserts WHERE UUID IS NOT NULL) AND UUID NOT IN (SELECT UUID FROM tblIntroductionPuzzleInserts WHERE UUID IS NOT NULL) AND Day>=? AND Found='true' ORDER BY tblIdentity.IdentityID, Day DESC, RequestIndex DESC;");
	st.Bind(0, Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	st.Step();

	if(st.RowReturned()==false)
	{
		content+="<td colspan=\"4\"><center>"+m_trans->Get("web.page.announceidentity.waitforpuzzles")+"</td>";
	}
	else
	{
		while(st.RowReturned() && shown<20)
		{
			st.ResultText(0,uuid);
			st.ResultText(1,day);
			st.ResultText(2,thisid);
			st.ResultInt(3,requestindex);
			st.ResultText(4,name);
			st.ResultText(5,pubkey);
			st.ResultText(6,captchatype);

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

			if(willshow && thisid!=lastid && (captchatype=="captcha" || captchatype=="unlikecaptcha1" || captchatype=="audiocaptcha1"))
			{
				StringFunctions::Convert(shown,countstr);
				if(shown>0 && shown%4==0)
				{
					content+="</tr>\r\n<tr>";
				}
				content+="<td style=\"vertical-align:top;\" title=\""+m_trans->Get("web.page.announceidentity.from")+" "+SanitizeOutput(CreateShortIdentityName(name,pubkey))+"\">";
				if(captchatype=="captcha" || captchatype=="unlikecaptcha1")
				{
					content+="<img src=\"showcaptcha.htm?UUID="+uuid+"\"><br />";
				}
				else
				{
					content+="<a href=\"showcaptcha.htm?UUID="+uuid+"\">"+m_trans->Get("web.page.announceidentity.listen")+"</a><br />";
				}
				content+="<input type=\"hidden\" name=\"uuid["+countstr+"]\" value=\""+SanitizeOutput(uuid)+"\">";
				content+="<input type=\"hidden\" name=\"day["+countstr+"]\" value=\""+day+"\">";
				content+="<input type=\"hidden\" name=\"captchatype["+countstr+"]\" value=\""+SanitizeOutput(captchatype)+"\">";
				content+="<input type=\"hidden\" name=\"identityid["+countstr+"]\" value=\""+thisid+"\">";
				if(captchatype=="captcha")
				{
					content+="<span class=\"small90\">"+m_trans->Get("web.page.announceidentity.charactercaptcha.instructions")+"</span><br />";
					content+="<input type=\"text\" name=\"solution["+countstr+"]\">";
				}
				else if(captchatype=="unlikecaptcha1")
				{
					content+="<span class=\"small90\">"+m_trans->Get("web.page.announceidentity.unlikecaptcha1.whichpanelunlike")+"</span><br />";
					content+="<select name=\"unlikenum["+countstr+"]\">";
					content+="<option value=\"1\">1</option>";
					content+="<option value=\"2\">2</option>";
					content+="<option value=\"3\">3</option>";
					content+="<option value=\"4\">4</option>";
					content+="</select>";
					content+="<br />";
					content+="<span class=\"small90\">"+m_trans->Get("web.page.announceidentity.unlikecaptcha1.unlikeobject")+"</span><br />";
					content+="<input type=\"text\" name=\"unlikeobject["+countstr+"]\">";
					content+="<br />";
					content+="<span class=\"small90\">"+m_trans->Get("web.page.announceidentity.unlikecaptcha1.similarobject")+"</span><br />";
					content+="<input type=\"text\" name=\"similarobject["+countstr+"]\">";
				}
				else if(captchatype=="audiocaptcha1")
				{
					content+="<span class=\"small90\">"+m_trans->Get("web.page.announceidentity.audiocaptcha.instructions")+"</span><br />";
					content+="<input type=\"text\" name=\"solution["+countstr+"]\">";
				}
				content+="</td>\r\n";
				lastid=thisid;
				shown++;
			}
			
			st.Step();
		}

		std::string mltlt("0");
		st=m_db->Prepare("SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust';");
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,mltlt);
		}

		content+="</tr>\r\n";
		content+="<tr><td colspan=\"2\" style=\"text-align:right;\"><input type=\"checkbox\" name=\"chkaddtrust\" value=\"Y\" checked></td><td colspan=\"2\">"+m_trans->Get("web.page.announceidentity.chkaddtrust")+"</td></tr>\r\n";
		content+="<tr><td colspan=\"2\" style=\"text-align:right;\"><input type=\"textbox\" name=\"txtaddtrust\" value=\""+mltlt+"\" size=\"2\" maxlength=\"3\"></td><td colspan=\"2\">"+m_trans->Get("web.page.announceidentity.txtaddtrust")+"</td></tr>\r\n";
		content+="<tr><td colspan=\"4\"><center><input type=\"submit\" value=\""+m_trans->Get("web.page.announceidentity.announce")+"\"></center></td>";
		
	}

	content+="</tr>\r\n";
	content+="</table>";
	content+="</form>";

	return content;
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
