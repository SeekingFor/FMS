#include "../../../include/http/pages/peertrustpage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string PeerTrustPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	int count=0;
	std::string countstr;
	std::string content="";
	int identitycount=0;		// total number of ids we know
	int rowsperpage=25;			// how many ids to show per page
	std::string rowsperpagestr;
	int startrow=0;
	std::string startrowstr="0";

	StringFunctions::Convert(rowsperpage,rowsperpagestr);

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="update")
	{
		std::vector<std::string> identityids;
		std::vector<std::string> oldlmt;
		std::vector<std::string> lmt;
		std::vector<std::string> oldltlt;
		std::vector<std::string> ltlt;
		int localmessagetrust=0;
		int localtrustlisttrust=0;
		int identityid;

		CreateArgArray(queryvars,"identityid",identityids);
		CreateArgArray(queryvars,"oldlocalmessagetrust",oldlmt);
		CreateArgArray(queryvars,"localmessagetrust",lmt);
		CreateArgArray(queryvars,"oldlocaltrustlisttrust",oldltlt);
		CreateArgArray(queryvars,"localtrustlisttrust",ltlt);
		
		SQLite3DB::Statement update=m_db->Prepare("UPDATE tblIdentity SET LocalMessageTrust=?, LocalTrustListTrust=? WHERE IdentityID=?;");

		for(int i=0; i<identityids.size(); i++)
		{
			if(oldlmt[i]!=lmt[i] || oldltlt[i]!=ltlt[i])
			{
				StringFunctions::Convert(lmt[i],localmessagetrust);
				StringFunctions::Convert(ltlt[i],localtrustlisttrust);
				StringFunctions::Convert(identityids[i],identityid);

				update.Bind(0,localmessagetrust);
				update.Bind(1,localtrustlisttrust);
				update.Bind(2,identityid);
				update.Step();
				update.Reset();
			}
		}

	}

	// if startrow is specified
	if(queryvars.find("startrow")!=queryvars.end())
	{
		startrowstr=(*queryvars.find("startrow")).second;
		// convert back and forth, just in case a number wasn't passed in startrow
		StringFunctions::Convert(startrowstr,startrow);
		if(startrow<0)
		{
			startrow=0;
		}
		StringFunctions::Convert(startrow,startrowstr);
	}

	content+="<h2>Peer Trust</h2>";
	content+="Message Trust is how much you trust the identity to post good messages. Trust List Trust is how much weight you want the trust list of that identity to have when calculating the total. The local trust levels are set by you, and the peer trust levels are calculated by a weighted average using other identities trust lists.";
	content+="<form name=\"frmtrust\" method=\"POST\">";
	content+="<input type=\"hidden\" name=\"formaction\" value=\"update\">";
	content+="<input type=\"hidden\" name=\"startrow\" value=\""+startrowstr+"\">";
	content+="<table>";
	content+="<tr><th>Name</th><th>Local Message Trust</th><th>Peer Message Trust</th><th>Local Trust List Trust</th><th>Peer Trust List Trust</th></tr>";
	
	SQLite3DB::Statement st=m_db->Prepare("SELECT COUNT(*) FROM tblIdentity;");
	st.Step();
	st.ResultInt(0,identitycount);
	st.Finalize();

	st=m_db->Prepare("SELECT IdentityID,Name,LocalMessageTrust,PeerMessageTrust,LocalTrustListTrust,PeerTrustListTrust,PublicKey FROM tblIdentity ORDER BY Name COLLATE NOCASE LIMIT "+startrowstr+","+rowsperpagestr+";");
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

		StringFunctions::Convert(count,countstr);

		st.ResultText(0,identityid);
		st.ResultText(1,name);
		st.ResultText(2,localmessagetrust);
		st.ResultText(3,peermessagetrust);
		st.ResultText(4,localtrustlisttrust);
		st.ResultText(5,peertrustlisttrust);
		st.ResultText(6,publickey);

		content+="<tr>";
		content+="<td title=\""+publickey+"\">";
		content+="<input type=\"hidden\" name=\"identityid["+countstr+"]\" value=\""+identityid+"\">";
		if(name!="")
		{
			content+=name;
		}
		else
		{
			content+="[Unknown Name]";
		}
		content+="</td>";
		content+="<td "+GetClassString(localmessagetrust)+">";
		content+="<input type=\"hidden\" name=\"oldlocalmessagetrust["+countstr+"]\" value=\""+localmessagetrust+"\">";
		content+="<input type=\"text\" name=\"localmessagetrust["+countstr+"]\" value=\""+localmessagetrust+"\" size=\"2\" maxlength=\"3\"></td>";
		content+="<td "+GetClassString(peermessagetrust)+">";
		content+=peermessagetrust+"</td>";
		content+="<td "+GetClassString(localtrustlisttrust)+">";
		content+="<input type=\"hidden\" name=\"oldlocaltrustlisttrust["+countstr+"]\" value=\""+localtrustlisttrust+"\">";
		content+="<input type=\"text\" name=\"localtrustlisttrust["+countstr+"]\" value=\""+localtrustlisttrust+"\" size=\"2\" maxlength=\"3\"></td>";
		content+="<td "+GetClassString(peertrustlisttrust)+">";
		content+=peertrustlisttrust+"</td>";
		content+="</tr>";
		st.Step();
		count++;
	}
	
	if(startrow>0 || startrow+rowsperpage<identitycount)
	{
		int tempint;
		std::string tempstr;
		int cols=0;

		content+="<tr>";
		if(startrow>0)
		{
			StringFunctions::Convert(startrow-rowsperpage,tempstr);
			content+="<td colspan=\"2\" align=\"left\"><a href=\"peertrust.htm?startrow="+tempstr+"\"><-- Previous Page</a></td>";
			cols+=2;
		}
		if(startrow+rowsperpage<identitycount)
		{
			StringFunctions::Convert(startrow+rowsperpage,tempstr);
			while(cols<3)
			{
				content+="<td></td>";
				cols++;
			}
			content+="<td colspan=\"2\" align=\"right\"><a href=\"peertrust.htm?startrow="+tempstr+"\">Next Page --></a></td>";
		}
		content+="</tr>";
	}

	content+="<tr><td colspan=\"5\"><input type=\"submit\" value=\"Update Trust\"></td></tr>";
	content+="</table>";
	content+="</form>";

	return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"+StringFunctions::Replace(m_template,"[CONTENT]",content);
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

const bool PeerTrustPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("peertrust.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
