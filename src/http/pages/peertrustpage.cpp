#include "../../../include/http/pages/peertrustpage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/global.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string PeerTrustPage::BuildQueryString(const long startrow, const std::string &namesearch, const std::string &sortby, const std::string &sortorder, const int localidentityid)
{
	std::string returnval="";
	std::string tempval="";
	
	if(startrow>=0)
	{
		StringFunctions::Convert(startrow,tempval);
		returnval+="startrow="+tempval;
	}

	if(namesearch!="")
	{
		if(returnval!="")
		{
			returnval+="&";
		}
		returnval+="namesearch="+StringFunctions::UriEncode(namesearch);
	}

	if(sortby!="")
	{
		if(returnval!="")
		{
			returnval+="&";
		}
		returnval+="sortby="+sortby;
	}

	if(sortorder!="")
	{
		if(returnval!="")
		{
			returnval+="&";
		}
		returnval+="sortorder="+sortorder;
	}

	if(localidentityid>=0)
	{
		std::string localidentityidstr="";
		StringFunctions::Convert(localidentityid,localidentityidstr);
		if(returnval!="")
		{
			returnval+="&";
		}
		returnval+="localidentityid="+localidentityidstr;
	}

	return returnval;

}

const std::string PeerTrustPage::CreateLocalIdentityDropDown(const std::string &name, const int selectedlocalidentityid)
{
	std::string result="";
	
	result+="<select name=\""+name+"\">";
	
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID,Name,PublicKey FROM tblLocalIdentity WHERE PublicKey IS NOT NULL ORDER BY Name COLLATE NOCASE;");
	st.Step();

	while(st.RowReturned())
	{
		int localidentityid=-1;
		std::string localidentityidstr="";
		std::string name="";
		std::string publickey="";

		st.ResultInt(0,localidentityid);
		st.ResultText(1,name);
		st.ResultText(2,publickey);

		StringFunctions::Convert(localidentityid,localidentityidstr);

		result+="<option value=\""+localidentityidstr+"\"";
		if(localidentityid==selectedlocalidentityid)
		{
			result+=" SELECTED";
		}
		result+=">"+SanitizeOutput(CreateShortIdentityName(name,publickey))+"</option>";
		st.Step();
	}

	result+="</select>";

	return result;
}

const std::string PeerTrustPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	int count=0;
	std::string countstr;
	std::string content="";
	int identitycount=0;		// total number of ids we know
	int rowsperpage=25;			// how many ids to show per page
	std::string rowsperpagestr;
	int startrow=0;
	std::string startrowstr="0";
	std::string namesearch="";
	std::string sql;
	std::string sortby="";
	std::string sortorder="";
	std::string localidentityidstr="";
	int localidentityid=-1;

	StringFunctions::Convert(rowsperpage,rowsperpagestr);

	// get localidentityid from querystring or load one from the database
	if(queryvars.find("localidentityid")!=queryvars.end())
	{
		localidentityidstr=(*queryvars.find("localidentityid")).second.GetData();
		StringFunctions::Convert(localidentityidstr,localidentityid);
		// insert the ID into the temporary table so we remember the identity if we load the page later
		m_db->Execute("DELETE FROM tmpLocalIdentityPeerTrustPage;");
		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tmpLocalIdentityPeerTrustPage(LocalIdentityID) VALUES(?);");
		st.Bind(0,localidentityid);
		st.Step();
	}
	else
	{
		// try to get the localidentityid if it exists in the temp table, otherwise load the first identity in the database
		SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tmpLocalIdentityPeerTrustPage;");
		st.Step();
		if(st.RowReturned())
		{
			st.ResultInt(0,localidentityid);
			StringFunctions::Convert(localidentityid,localidentityidstr);
		}
		else
		{
			st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity;");
			st.Step();
			if(st.RowReturned())
			{
				st.ResultInt(0,localidentityid);
				StringFunctions::Convert(localidentityid,localidentityidstr);
			}
			st.Finalize();
		}
	}

	if(localidentityid!=-1 && queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="update" && ValidateFormPassword(queryvars))
	{
		std::vector<std::string> identityids;
		std::vector<std::string> oldlmt;
		std::vector<std::string> lmt;
		std::vector<std::string> oldltlt;
		std::vector<std::string> ltlt;
		std::vector<std::string> oldmtc;
		std::vector<std::string> mtc;
		std::vector<std::string> oldtltc;
		std::vector<std::string> tltc;
		int localmessagetrust=0;
		int localtrustlisttrust=0;
		int identityid=-1;

		CreateArgArray(queryvars,"identityid",identityids);
		CreateArgArray(queryvars,"oldlocalmessagetrust",oldlmt);
		CreateArgArray(queryvars,"localmessagetrust",lmt);
		CreateArgArray(queryvars,"oldlocaltrustlisttrust",oldltlt);
		CreateArgArray(queryvars,"localtrustlisttrust",ltlt);
		CreateArgArray(queryvars,"oldmessagetrustcomment",oldmtc);
		CreateArgArray(queryvars,"messagetrustcomment",mtc);
		CreateArgArray(queryvars,"oldtrustlisttrustcomment",oldtltc);
		CreateArgArray(queryvars,"trustlisttrustcomment",tltc);

		SQLite3DB::Statement ins=m_db->Prepare("INSERT INTO tblIdentityTrust(LocalIdentityID,IdentityID) VALUES(?,?);");
		SQLite3DB::Statement update=m_db->Prepare("UPDATE tblIdentityTrust SET LocalMessageTrust=?, LocalTrustListTrust=?, MessageTrustComment=?, TrustListTrustComment=? WHERE LocalIdentityID=? AND IdentityID=?;");

		for(int i=0; i<identityids.size(); i++)
		{
			if(oldlmt[i]!=lmt[i] || oldltlt[i]!=ltlt[i] || oldmtc[i]!=mtc[i] || oldtltc[i]!=tltc[i])
			{
				StringFunctions::Convert(lmt[i],localmessagetrust);
				StringFunctions::Convert(ltlt[i],localtrustlisttrust);
				StringFunctions::Convert(identityids[i],identityid);

				ins.Bind(0,localidentityid);
				ins.Bind(1,identityid);
				ins.Step();
				ins.Reset();

				if(lmt[i]!="")
				{
					update.Bind(0,localmessagetrust);
				}
				else
				{
					update.Bind(0);
				}
				if(ltlt[i]!="")
				{
					update.Bind(1,localtrustlisttrust);
				}
				else
				{
					update.Bind(1);
				}
				update.Bind(2,mtc[i]);
				update.Bind(3,tltc[i]);
				update.Bind(4,localidentityid);
				update.Bind(5,identityid);
				update.Step();
				update.Reset();
			}
		}

	}

	// if startrow is specified
	if(queryvars.find("startrow")!=queryvars.end())
	{
		startrowstr=(*queryvars.find("startrow")).second.GetData();
		// convert back and forth, just in case a number wasn't passed in startrow
		StringFunctions::Convert(startrowstr,startrow);
		if(startrow<0)
		{
			startrow=0;
		}
		StringFunctions::Convert(startrow,startrowstr);
	}

	// sort by
	if(queryvars.find("sortby")!=queryvars.end())
	{
		sortby=(*queryvars.find("sortby")).second.GetData();
		if(sortby!="Name" && sortby!="tblIdentityTrust.LocalMessageTrust" && sortby!="PeerMessageTrust" && sortby!="tblIdentityTrust.LocalTrustListTrust" && sortby!="PeerTrustListTrust" && sortby!="MessageCount")
		{
			sortby="Name";
		}
	}
	else
	{
		sortby="Name";
	}

	// sort order
	if(queryvars.find("sortorder")!=queryvars.end())
	{
		sortorder=(*queryvars.find("sortorder")).second.GetData();
		if(sortorder!="ASC" && sortorder!="DESC")
		{
			sortorder="ASC";
		}
	}
	else
	{
		sortorder="ASC";
	}

	// if we are searching by name
	if(queryvars.find("namesearch")!=queryvars.end())
	{
		namesearch=(*queryvars.find("namesearch")).second.GetData();
	}

	content+="<h2>"+m_trans->Get("web.page.peertrust.title")+"</h2>";
	content+=m_trans->Get("web.page.peertrust.instructions")+"<br>";
	content+=m_trans->Get("web.page.peertrust.notrustlist")+"<br>";

	// search drop down
	content+="<div style=\"text-align:center;margin-bottom:5px;\">";
	content+="<form name=\"frmsearch\" method=\"POST\" action=\""+m_pagename+"?"+BuildQueryString(0,"","","",localidentityid)+"\">";
	content+="<input type=\"text\" name=\"namesearch\" value=\""+SanitizeOutput(namesearch)+"\">";
	content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peertrust.search")+"\">";
	content+="</form>";
	content+="</div>";

	content+="<div style=\"text-align:center;\">";
	content+="<form name=\"frmlocalidentity\" method=\"POST\" action=\""+m_pagename+"?"+BuildQueryString(startrow,namesearch,sortby,sortorder,-1)+"\">";
	content+=m_trans->Get("web.page.peertrust.loadtrustlist")+" ";
	content+=CreateLocalIdentityDropDown("localidentityid",localidentityid);
	content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peertrust.loadlist")+"\">";
	content+="</form>";
	content+="</div>";

	content+="<form name=\"frmtrust\" method=\"POST\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"update\">";
	content+="<input type=\"hidden\" name=\"localidentityid\" value=\""+localidentityidstr+"\">";
	content+="<input type=\"hidden\" name=\"startrow\" value=\""+startrowstr+"\">";
	if(namesearch!="")
	{
		content+="<input type=\"hidden\" name=\"namesearch\" value=\""+SanitizeOutput(namesearch)+"\">";
	}
	content+="<table class=\"small90\">";
	content+="<tr><th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,namesearch,"Name",ReverseSort("Name",sortby,sortorder),localidentityid)+"\">"+m_trans->Get("web.page.peertrust.name")+"</a></th>";
	content+="<th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,namesearch,"tblIdentityTrust.LocalMessageTrust",ReverseSort("tblIdentityTrust.LocalMessageTrust",sortby,sortorder),localidentityid)+"\">"+m_trans->Get("web.page.peertrust.localmessagetrust")+"</a></th>";
	content+="<th>"+m_trans->Get("web.page.peertrust.messagecomment")+"</th>";
	content+="<th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,namesearch,"PeerMessageTrust",ReverseSort("PeerMessageTrust",sortby,sortorder),localidentityid)+"\">"+m_trans->Get("web.page.peertrust.peermessagetrust")+"</a></th>";
	content+="<th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,namesearch,"tblIdentityTrust.LocalTrustListTrust",ReverseSort("tblIdentityTrust.LocalTrustListTrust",sortby,sortorder),localidentityid)+"\">"+m_trans->Get("web.page.peertrust.localtrustlisttrust")+"</a></th>";
	content+="<th>"+m_trans->Get("web.page.peertrust.trustcomment")+"</th>";
	content+="<th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,namesearch,"PeerTrustListTrust",ReverseSort("PeerTrustListTrust",sortby,sortorder),localidentityid)+"\">"+m_trans->Get("web.page.peertrust.peertrustlisttrust")+"</a></th>";
	content+="<th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,namesearch,"MessageCount",ReverseSort("MessageCount",sortby,sortorder),localidentityid)+"\">"+m_trans->Get("web.page.peertrust.messagecount")+"</a></th>";
	content+="</tr>\r\n";
	
	// get count of identities we are showing
	sql="SELECT COUNT(*) FROM tblIdentity LEFT JOIN tblIdentityTrust ON tblIdentity.IdentityID=tblIdentityTrust.IdentityID ";
	sql+="WHERE tblIdentityTrust.LocalIdentityID=? AND tblIdentity.Hidden='false'";
	if(namesearch!="")
	{
		sql+=" AND (Name LIKE '%' || ? || '%' OR PublicKey LIKE '%' || ? || '%')";
	}
	sql+=";";
	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Bind(0,localidentityid);
	if(namesearch!="")
	{
		st.Bind(1,namesearch);
		st.Bind(2,namesearch);
	}
	st.Step();
	st.ResultInt(0,identitycount);
	st.Finalize();

	sql="SELECT tblIdentity.IdentityID,Name,tblIdentityTrust.LocalMessageTrust,PeerMessageTrust,tblIdentityTrust.LocalTrustListTrust,PeerTrustListTrust,PublicKey,tblIdentityTrust.MessageTrustComment,tblIdentityTrust.TrustListTrustComment,COUNT(MessageID) AS 'MessageCount',tblIdentity.PublishTrustList ";
	sql+="FROM tblIdentity LEFT JOIN tblIdentityTrust ON tblIdentity.IdentityID=tblIdentityTrust.IdentityID LEFT JOIN tblMessage ON tblIdentity.IdentityID=tblMessage.IdentityID ";
	sql+="WHERE tblIdentityTrust.LocalIdentityID=? AND tblIdentity.Hidden='false'";
	if(namesearch!="")
	{
		sql+=" AND (Name LIKE  '%' || ? || '%' OR PublicKey LIKE '%' || ? || '%')";
	}
	sql+=" GROUP BY tblIdentity.IdentityID";
	sql+=" ORDER BY";
	if(sortby=="Name")
	{
		sql+=" Name COLLATE NOCASE";
	}
	else
	{
		sql+=" "+sortby;
	}
	if(sortorder!="")
	{
		sql+=" "+sortorder;
	}
	sql+=" LIMIT "+startrowstr+","+rowsperpagestr+";";
	st=m_db->Prepare(sql);
	st.Bind(0,localidentityid);
	if(namesearch!="")
	{
		st.Bind(1,namesearch);
		st.Bind(2,namesearch);
	}
	st.Step();

	while(st.RowReturned())
	{
		std::string identityid;
		std::string name;
		std::string localmessagetrust;
		std::string peermessagetrust;
		std::string localtrustlisttrust;
		std::string peertrustlisttrust;
		std::string publickey;
		std::string messagetrustcomment="";
		std::string trustlisttrustcomment="";
		std::string messagecountstr="";
		std::string publishtrustlist="";

		StringFunctions::Convert(count,countstr);

		st.ResultText(0,identityid);
		st.ResultText(1,name);
		st.ResultText(2,localmessagetrust);
		st.ResultText(3,peermessagetrust);
		st.ResultText(4,localtrustlisttrust);
		st.ResultText(5,peertrustlisttrust);
		st.ResultText(6,publickey);
		st.ResultText(7,messagetrustcomment);
		st.ResultText(8,trustlisttrustcomment);
		st.ResultText(9,messagecountstr);
		st.ResultText(10,publishtrustlist);

		content+="<tr>";
		content+="<td title=\""+publickey+"\">";
		content+="<input type=\"hidden\" name=\"identityid["+countstr+"]\" value=\""+identityid+"\">";
		content+="<a href=\"peerdetails.htm?identityid="+identityid+"\">";
		if(name!="")
		{
			content+=SanitizeOutput(CreateShortIdentityName(name,publickey));
		}
		else
		{
			content+="[Unknown Name]";
		}
		content+="</a>";
		content+="</td>";
		content+="<td "+GetClassString(localmessagetrust)+">";
		content+="<input type=\"hidden\" name=\"oldlocalmessagetrust["+countstr+"]\" value=\""+localmessagetrust+"\">";
		content+="<input type=\"text\" name=\"localmessagetrust["+countstr+"]\" value=\""+localmessagetrust+"\" size=\"2\" maxlength=\"3\" class=\"small90\"></td>";
		content+="<td "+GetClassString(localmessagetrust)+">";
		content+="<input type=\"hidden\" name=\"oldmessagetrustcomment["+countstr+"]\" value=\""+SanitizeOutput(messagetrustcomment)+"\">";
		content+="<input type=\"text\" name=\"messagetrustcomment["+countstr+"]\" value=\""+SanitizeOutput(messagetrustcomment)+"\" maxlength=\"50\" class=\"small90\">";
		content+="</td>";		
		content+="<td "+GetClassString(peermessagetrust)+">";
		content+=peermessagetrust+"</td>";
		content+="<td "+GetClassString(localtrustlisttrust)+">";
		content+="<input type=\"hidden\" name=\"oldlocaltrustlisttrust["+countstr+"]\" value=\""+localtrustlisttrust+"\">";
		content+="<input type=\"text\" name=\"localtrustlisttrust["+countstr+"]\" value=\""+localtrustlisttrust+"\" size=\"2\" maxlength=\"3\" class=\"small90\">";
		if(publishtrustlist=="false")
		{
			content+="*";
		}
		content+="</td>";
		content+="<td "+GetClassString(localtrustlisttrust)+">";
		content+="<input type=\"hidden\" name=\"oldtrustlisttrustcomment["+countstr+"]\" value=\""+SanitizeOutput(trustlisttrustcomment)+"\">";
		content+="<input type=\"text\" name=\"trustlisttrustcomment["+countstr+"]\" value=\""+SanitizeOutput(trustlisttrustcomment)+"\" maxlength=\"50\" class=\"small90\">";
		content+="</td>";
		content+="<td "+GetClassString(peertrustlisttrust)+">";
		content+=peertrustlisttrust+"</td>";
		content+="<td>"+messagecountstr+"</td>";
		content+="</tr>\r\n";
		st.Step();
		count++;
	}
	
	if(startrow>0 || startrow+rowsperpage<identitycount)
	{
		std::string tempstr;
		int cols=0;

		content+="<tr>";
		if(startrow>0)
		{
			StringFunctions::Convert(startrow-rowsperpage,tempstr);
			content+="<td colspan=\"3\" align=\"left\"><a href=\""+m_pagename+"?"+BuildQueryString(startrow-rowsperpage,namesearch,sortby,sortorder,localidentityid)+"\">"+m_trans->Get("web.page.peertrust.previouspage")+"</a></td>";
			cols+=3;
		}
		if(startrow+rowsperpage<identitycount)
		{
			while(cols<5)
			{
				content+="<td></td>";
				cols++;
			}
			content+="<td colspan=\"3\" align=\"right\"><a href=\""+m_pagename+"?"+BuildQueryString(startrow+rowsperpage,namesearch,sortby,sortorder,localidentityid)+"\">"+m_trans->Get("web.page.peertrust.nextpage")+"</a></td>";
		}
		content+="</tr>";
	}

	content+="<tr><td colspan=\"8\"><center><input type=\"submit\" value=\""+m_trans->Get("web.page.peertrust.updatetrust")+"\"></center></td></tr>";
	content+="</table>";
	content+="</form>";

	return content;
}

const std::string PeerTrustPage::GetClassString(const std::string &trustlevel)
{
	int tempint=0;
	std::string tempstr;

	StringFunctions::Convert(trustlevel,tempint);
	tempint/=10;
	StringFunctions::Convert(tempint,tempstr);

	if(trustlevel!="")
	{
		return "class=\"trust"+tempstr+"\"";
	}
	else
	{
		return "";
	}
}

const std::string PeerTrustPage::ReverseSort(const std::string &sortname, const std::string &currentsortby, const std::string &currentsortorder)
{
	if(sortname==currentsortby)
	{
		if(currentsortorder=="ASC")
		{
			return "DESC";
		}
		else
		{
			return "ASC";
		}
	}
	else
	{
		return currentsortorder;
	}
}
