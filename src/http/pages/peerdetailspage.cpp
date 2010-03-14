#include "../../../include/http/pages/peerdetailspage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/option.h"
#include "../../../include/global.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string PeerDetailsPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	std::string content="";
	int identityid=0;
	std::string identityidstr="";
	std::string name;
	std::string publickey;
	std::string messagetrust;
	std::string trustlisttrust;
	std::string lastseen="";
	std::string dateadded="";
	std::string addedmethod="";
	std::string usk="";
	std::string fproxyhost="127.0.0.1";
	std::string fproxyprotocol("http");
	std::string hidden="";
	int freesiteedition=-1;
	std::string publishtrustlist="";
	std::string messagebase="";

	Option option(m_db);
	std::string fproxyport="8888";
	option.Get("FProxyPort",fproxyport);
	option.Get("MessageBase",messagebase);
	option.Get("FProxyHost",fproxyhost);
	option.Get("FProxyProtocol",fproxyprotocol);

	if(queryvars.find("identityid")!=queryvars.end() && (*queryvars.find("identityid")).second!="")
	{
		identityidstr=(*queryvars.find("identityid")).second.GetData();
		StringFunctions::Convert((*queryvars.find("identityid")).second.GetData(),identityid);
	}

	if(identityid!=0 && queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="deletemessages" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement del=m_db->Prepare("DELETE FROM tblMessage WHERE IdentityID=?;");
		del.Bind(0,identityid);
		del.Step();
	}

	if(identityid!=0 && queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="hide" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement del=m_db->Prepare("UPDATE tblIdentity SET Hidden='true' WHERE IdentityID=?;");
		del.Bind(0,identityid);
		del.Step();
	}
	
	if(identityid!=0 && queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="show" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement del=m_db->Prepare("UPDATE tblIdentity SET Hidden='false' WHERE IdentityID=?;");
		del.Bind(0,identityid);
		del.Step();
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT Name,PublicKey,DateAdded,LastSeen,AddedMethod,Hidden,FreesiteEdition,PublishTrustList FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,identityid);
	st.Step();

	content+="<table>";
	if(st.RowReturned())
	{
		st.ResultText(0,name);
		st.ResultText(1,publickey);
		st.ResultText(2,dateadded);
		st.ResultText(3,lastseen);
		st.ResultText(4,addedmethod);
		st.ResultText(5,hidden);
		if(st.ResultNull(6)==false)
		{
			st.ResultInt(6,freesiteedition);
		}
		st.ResultText(7,publishtrustlist);

		usk=publickey;
		if(freesiteedition>=0 && usk.find("SSK@")==0)
		{
			std::string editionstr="";
			usk.erase(0,3);
			StringFunctions::Convert(freesiteedition,editionstr);
			usk="USK"+usk+messagebase+"/"+editionstr+"/";
		}
		else
		{
			usk="";
		}

		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.name")+"</td><td>"+SanitizeOutput(name)+"</td></tr>";
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.publickey")+"</td><td class=\"smaller\">"+SanitizeOutput(publickey)+"</td></tr>";
		if(usk!="")
		{
			content+="<tr><td>"+m_trans->Get("web.page.peerdetails.freesite")+"</td><td class=\"smaller\"><a href=\""+fproxyprotocol+"://"+fproxyhost+":"+fproxyport+"/"+SanitizeOutput(usk)+"\">"+SanitizeOutput(usk)+"</a></td></tr>";
		}
		if(publishtrustlist=="true")
		{
			std::string lastseendate=lastseen;
			if(lastseendate.size()>=10)
			{
				lastseendate=lastseendate.substr(0,10);
			}
			content+="<tr><td>"+m_trans->Get("web.page.peerdetails.trustlistxml")+"</td><td class=\"smaller\"><a href=\""+fproxyprotocol+"://"+fproxyhost+":"+fproxyport+"/"+SanitizeOutput(publickey)+messagebase+"|"+lastseendate+"|TrustList|0.xml\">"+m_trans->Get("web.page.peerdetails.trustlist")+"</a></td></tr>";
		}
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.dateadded")+"</td><td>"+dateadded+"</td></tr>";
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.lastseen")+"</td><td>"+lastseen+"</td></tr>";
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.addedmethod")+"</td><td class=\"smaller\">"+SanitizeOutput(addedmethod)+"</td></tr>";
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.hiddeninpeertrust")+"</td>";
		content+="<td>"+hidden;
		content+="&nbsp;<form name=\"frmhidden\" method=\"POST\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"identityid\" value=\""+identityidstr+"\">";
		if(hidden=="false")
		{
			content+="<input type=\"hidden\" name=\"formaction\" value=\"hide\">";
			content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peerdetails.hide")+"\">";
		}
		else
		{
			content+="<input type=\"hidden\" name=\"formaction\" value=\"show\">";
			content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peerdetails.show")+"\">";
		}
		content+="</form>";
		content+="</td></tr>";
	}

	// get message count posted by this identity
	st=m_db->Prepare("SELECT COUNT(MessageID) FROM tblMessage WHERE IdentityID=?;");
	st.Bind(0,identityid);
	st.Step();

	if(st.RowReturned())
	{
		std::string messagecountstr="0";
		st.ResultText(0,messagecountstr);
		content+="<tr>";
		content+="<td>"+m_trans->Get("web.page.peerdetails.messagecount")+"</td>";
		content+="<td>"+messagecountstr;
		content+="&nbsp;&nbsp;<form name=\"frmdeletemessages\" method=\"POST\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"identityid\" value=\""+identityidstr+"\">";
		content+="<input type=\"hidden\" name=\"formaction\" value=\"deletemessages\">";
		content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peerdetails.deletemessages")+"\">";
		content+="</form>";
		content+="</td>";
		content+="</tr>";
	}

	content+="</table>";


	st=m_db->Prepare("SELECT Name,PublicKey,MessageTrust,TrustListTrust,tblIdentity.IdentityID,tblPeerTrust.MessageTrustComment,tblPeerTrust.TrustListTrustComment FROM tblPeerTrust INNER JOIN tblIdentity ON tblPeerTrust.TargetIdentityID=tblIdentity.IdentityID WHERE tblPeerTrust.IdentityID=? ORDER BY Name COLLATE NOCASE;");
	st.Bind(0,identityid);
	st.Step();

	content+="<table>";
	content+="<tr><th colspan=\"5\">";
	content+=m_trans->Get("web.page.peerdetails.trustlistofthisidentity");
	content+="</th></tr>";
	content+="<tr><td></td><th>"+m_trans->Get("web.page.peerdetails.messagetrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.messagecomment")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlisttrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlistcomment")+"</th></tr>";
	while(st.RowReturned())
	{
		std::string thisid="";
		std::string messagetrustcomment="";
		std::string trustlisttrustcomment="";

		st.ResultText(0,name);
		st.ResultText(1,publickey);
		st.ResultText(2,messagetrust);
		st.ResultText(3,trustlisttrust);
		st.ResultText(4,thisid);
		st.ResultText(5,messagetrustcomment);
		st.ResultText(6,trustlisttrustcomment);

		content+="<tr>";
		content+="<td><a href=\"peerdetails.htm?identityid="+thisid+"\">"+SanitizeOutput(CreateShortIdentityName(name,publickey))+"</a></td>";
		content+="<td "+GetClassString(messagetrust)+">"+messagetrust+"</td>";
		content+="<td>"+SanitizeOutput(messagetrustcomment)+"</td>";
		content+="<td "+GetClassString(trustlisttrust)+">"+trustlisttrust+"</td>";
		content+="<td>"+SanitizeOutput(trustlisttrustcomment)+"</td>";
		content+="</tr>\r\n";

		st.Step();
	}

	st=m_db->Prepare("SELECT Name,PublicKey,MessageTrust,TrustListTrust,tblIdentity.IdentityID,tblPeerTrust.MessageTrustComment,tblPeerTrust.TrustListTrustComment FROM tblPeerTrust INNER JOIN tblIdentity ON tblPeerTrust.IdentityID=tblIdentity.IdentityID WHERE tblPeerTrust.TargetIdentityID=? ORDER BY Name COLLATE NOCASE;");
	st.Bind(0,identityid);
	st.Step();

	content+="<tr><th colspan=\"5\"><hr></th></tr>";
	content+="<tr><th colspan=\"5\">";
	content+=m_trans->Get("web.page.peerdetails.trustofthisidentityfromotheridentities");
	content+="</th></tr>";
	content+="<tr><td></td><th>"+m_trans->Get("web.page.peerdetails.messagetrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.messagecomment")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlisttrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlistcomment")+"</th></tr>";
	while(st.RowReturned())
	{
		std::string thisid="";
		std::string messagetrustcomment="";
		std::string trustlisttrustcomment="";

		st.ResultText(0,name);
		st.ResultText(1,publickey);
		st.ResultText(2,messagetrust);
		st.ResultText(3,trustlisttrust);
		st.ResultText(4,thisid);
		st.ResultText(5,messagetrustcomment);
		st.ResultText(6,trustlisttrustcomment);

		content+="<tr>";
		content+="<td><a href=\"peerdetails.htm?identityid="+thisid+"\">"+SanitizeOutput(CreateShortIdentityName(name,publickey))+"</a></td>";
		content+="<td "+GetClassString(messagetrust)+">"+messagetrust+"</td>";
		content+="<td>"+SanitizeOutput(messagetrustcomment)+"</td>";
		content+="<td "+GetClassString(trustlisttrust)+">"+trustlisttrust+"</td>";
		content+="<td>"+SanitizeOutput(trustlisttrustcomment)+"</td>";
		content+="</tr>";

		st.Step();
	}
	content+="</table>";

	return content;
}

const std::string PeerDetailsPage::GetClassString(const std::string &trustlevel)
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

const bool PeerDetailsPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("peerdetails.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
