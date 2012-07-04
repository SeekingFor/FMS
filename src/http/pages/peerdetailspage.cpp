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
	std::string fproxyport="8888";
	std::string hidden="";
	int freesiteedition=-1;
	std::string publishtrustlist="";
	std::string messagebase="";
	int isfms=0;
	int iswot=0;
	bool showsignature=false;
	bool showavatar=false;
	std::string wotlastseen="";

	Option option(m_db);

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

	if(identityid!=0 && queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="hidesignature" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement del=m_db->Prepare("UPDATE tblIdentity SET ShowSignature=0 WHERE IdentityID=?;");
		del.Bind(0,identityid);
		del.Step();
	}
	
	if(identityid!=0 && queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="showsignature" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement del=m_db->Prepare("UPDATE tblIdentity SET ShowSignature=1 WHERE IdentityID=?;");
		del.Bind(0,identityid);
		del.Step();
	}

	if(identityid!=0 && queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="hideavatar" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement del=m_db->Prepare("UPDATE tblIdentity SET ShowAvatar=0 WHERE IdentityID=?;");
		del.Bind(0,identityid);
		del.Step();
	}
	
	if(identityid!=0 && queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="showavatar" && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement del=m_db->Prepare("UPDATE tblIdentity SET ShowAvatar=1 WHERE IdentityID=?;");
		del.Bind(0,identityid);
		del.Step();
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT Name,PublicKey,DateAdded,LastSeen,AddedMethod,Hidden,FreesiteEdition,PublishTrustList,PeerMessageTrust,PeerTrustListTrust,IsFMS,IsWOT,WOTLastSeen,ShowSignature,ShowAvatar FROM tblIdentity WHERE IdentityID=?;");
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
		st.ResultText(8,messagetrust);
		st.ResultText(9,trustlisttrust);
		st.ResultInt(10,isfms);
		st.ResultInt(11,iswot);
		st.ResultText(12,wotlastseen);
		st.ResultBool(13,showsignature);
		st.ResultBool(14,showavatar);

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
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.fmsidentity")+"</td><td>"+(isfms==1 ? m_trans->Get("web.page.peerdetails.yes") : m_trans->Get("web.page.peerdetails.no"))+"</td></tr>";
		if(isfms==1)
		{
			content+="<tr><td>"+m_trans->Get("web.page.peerdetails.lastseenfms")+"</td><td>"+lastseen+"</td></tr>";
		}
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.wotidentity")+"</td><td>"+(iswot==1 ? m_trans->Get("web.page.peerdetails.yes") : m_trans->Get("web.page.peerdetails.no"))+"</td></tr>";
		if(iswot==1)
		{
			content+="<tr><td>"+m_trans->Get("web.page.peerdetails.lastseenwot")+"</td><td>"+wotlastseen+"</td></tr>";
		}
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.addedmethod")+"</td><td class=\"smaller\">"+SanitizeOutput(addedmethod)+"</td></tr>";
		
		// list show/hide
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
		content+="</form></td></tr>";

		// signature show/hide
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.showforumsignature")+"</td><td>";
		content+="<form name=\"frmsignature\" method=\"POST\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"identityid\" value=\""+identityidstr+"\">";
		if(showsignature==true)
		{
			content+=m_trans->Get("web.page.peerdetails.yes")+"&nbsp;";
			content+="<input type=\"hidden\" name=\"formaction\" value=\"hidesignature\">";
			content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peerdetails.hide")+"\">";
		}
		else
		{
			content+=m_trans->Get("web.page.peerdetails.no")+"&nbsp;";
			content+="<input type=\"hidden\" name=\"formaction\" value=\"showsignature\">";
			content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peerdetails.show")+"\">";
		}
		content+="</form></td></tr>";

		// avatar show/hide
		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.showforumavatar")+"</td><td>";
		content+="<form name=\"frmavatar\" method=\"POST\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"identityid\" value=\""+identityidstr+"\">";
		if(showavatar==true)
		{
			content+=m_trans->Get("web.page.peerdetails.yes")+"&nbsp;";
			content+="<input type=\"hidden\" name=\"formaction\" value=\"hideavatar\">";
			content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peerdetails.hide")+"\">";
		}
		else
		{
			content+=m_trans->Get("web.page.peerdetails.no")+"&nbsp;";
			content+="<input type=\"hidden\" name=\"formaction\" value=\"showavatar\">";
			content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peerdetails.show")+"\">";
		}
		content+="</form></td></tr>";

		// Last received msg list
		st=m_db->Prepare("SELECT mlr.Day, mlr.RequestIndex FROM tblIdentity i, tblMessageListRequests mlr WHERE i.IdentityID=? AND i.IdentityID=mlr.IdentityID AND mlr.Found='true' ORDER by Day DESC, RequestIndex DESC LIMIT 1;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			std::string day="", idx="";
			st.ResultText(0,day);
			st.ResultText(1,idx);
			day = day.substr(0,4)+"."+day.substr(5,2)+"."+day.substr(8,2);
			dateadded = "<a href=\""+fproxyprotocol+"://"+fproxyhost+":"+fproxyport+"/USK"+publickey.substr(3)+messagebase+"|"+day+"|MessageList/"+idx+"/MessageList.xml?type=text/plain\">From "+day+", edition "+idx+"</a>";
		}
		else
		{
			dateadded = "<i>"+m_trans->Get("web.page.peerdetails.never")+"</i>";
		}

		content+="<tr><td>"+m_trans->Get("web.page.peerdetails.lastreceivedmessagelist")+"</td><td>"+dateadded+"</td></tr>";

		content+="</td></tr>";
		content+="<tr><td>"+m_trans->Get("web.page.peertrust.peermessagetrust")+"</td><td>"+messagetrust+"</td></tr>";
		content+="<tr><td>"+m_trans->Get("web.page.peertrust.peertrustlisttrust")+"</td><td>"+trustlisttrust+"</td></tr>";
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
		content+="&nbsp;&nbsp;(<a href=\"showreceivedmessage.htm?identityid="+identityidstr+"\">"+m_trans->Get("web.page.peerdetails.show")+"</a>)";
		content+="&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
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

	content+="<table class=\"small90\">";
	content+="<col align=\"right\"><col width=\"30\"><col><col width=\"30\"><col><col width=\"30\">";
	content+="<thead>";
	content+="<tr><th colspan=\"6\" align=\"center\"><a href=\"peertrust.htm?namesearch="+SanitizeOutput(publickey)+"\">";
	content+=m_trans->Get("web.page.peerdetails.localtrustofidentity");
	content+="</a></th></tr>";
	content+="<tr>";
	content+="<tr><th>"+m_trans->Get("web.page.peerdetails.localtrustlistof")+"</th><th>"+m_trans->Get("web.page.peerdetails.messagetrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.messagecomment")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlisttrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlistcomment")+"</th></tr>";
	content+="</tr>";
	content+="</thead>";

	content+="<tbody>";
	st=m_db->Prepare("SELECT LocalIdentityID,Name,PublicKey,LocalMessageTrust,MessageTrustComment,LocalTrustListTrust,TrustListTrustComment FROM tblLocalIdentity JOIN tblIdentityTrust USING (LocalIdentityID) WHERE IdentityID=? ORDER BY Name");
	st.Bind(0,identityid);
	st.Step();

	while(st.RowReturned())
	{
		std::string localidentityid="";
		std::string localname="";
		std::string localpublickey="";
		std::string messagetrustcomment="";
		std::string trustlisttrustcomment="";

		st.ResultText(0,localidentityid);
		st.ResultText(1,localname);
		st.ResultText(2,localpublickey);
		st.ResultText(3,messagetrust);
		st.ResultText(4,messagetrustcomment);
		st.ResultText(5,trustlisttrust);
		st.ResultText(6,trustlisttrustcomment);

		content+="<tr>";
		content+="<form method=\"POST\" action=\"peertrust.htm\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"formaction\" value=\"update\">";
		content+="<input type=\"hidden\" name=\"namesearch\" value=\""+SanitizeOutput(publickey)+"\">";
		content+="<input type=\"hidden\" name=\"localidentityid\" value=\""+localidentityid+"\">";
		content+="<input type=\"hidden\" name=\"identityid[0]\" value=\""+identityidstr+"\">";
		content+="<td title=\""+SanitizeOutput(localpublickey)+"\">"+SanitizeOutput(CreateShortIdentityName(localname,localpublickey))+"</td>";
		content+="<td>";
		content+="<input type=\"hidden\" name=\"oldlocalmessagetrust[0]\" value=\""+messagetrust+"\">";
		content+="<input type=\"text\" name=\"localmessagetrust[0]\" value=\""+messagetrust+"\" size=\"2\" maxlength=\"3\" class=\"small90\">";
		content+="</td><td>";
		content+="<input type=\"hidden\" name=\"oldmessagetrustcomment[0]\" value=\""+SanitizeOutput(messagetrustcomment)+"\">";
		content+="<input type=\"text\" name=\"messagetrustcomment[0]\" value=\""+SanitizeOutput(messagetrustcomment)+"\" maxlength=\"50\" class=\"small90\">";
		content+="</td><td>";
		content+="<input type=\"hidden\" name=\"oldlocaltrustlisttrust[0]\" value=\""+trustlisttrust+"\">";
		content+="<input type=\"text\" name=\"localtrustlisttrust[0]\" value=\""+trustlisttrust+"\" size=\"2\" maxlength=\"3\" class=\"small90\">";
		content+="</td><td>";
		content+="<input type=\"hidden\" name=\"oldtrustlisttrustcomment[0]\" value=\""+SanitizeOutput(trustlisttrustcomment)+"\">";
		content+="<input type=\"text\" name=\"trustlisttrustcomment[0]\" value=\""+SanitizeOutput(trustlisttrustcomment)+"\" maxlength=\"50\" class=\"small90\">";
		content+="</td><td>";
		content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.peertrust.updatetrust")+"\">";
		content+="</form>";
		content+="</td>";
		content+="</tr>";
		st.Step();
	}
	content+="</tbody>";
	content+="</table>";

	st=m_db->Prepare("SELECT Name,PublicKey,MessageTrust,TrustListTrust,tblIdentity.IdentityID,tblPeerTrust.MessageTrustComment,tblPeerTrust.TrustListTrustComment,MessageTrustChange,TrustListTrustChange FROM tblPeerTrust INNER JOIN tblIdentity ON tblPeerTrust.TargetIdentityID=tblIdentity.IdentityID WHERE tblPeerTrust.IdentityID=? ORDER BY Name COLLATE NOCASE;");
	st.Bind(0,identityid);
	st.Step();

	content+="<table class=\"small90\">";
	content+="<tr><th colspan=\"5\">";
	content+=m_trans->Get("web.page.peerdetails.trustlistofthisidentity");
	content+="</th></tr>";
	content+="<tr><td></td><th>"+m_trans->Get("web.page.peerdetails.messagetrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.lastchange")+"</th><th>"+m_trans->Get("web.page.peerdetails.messagecomment")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlisttrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.lastchange")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlistcomment")+"</th></tr>";
	while(st.RowReturned())
	{
		std::string thisid="";
		std::string messagetrustcomment="";
		std::string trustlisttrustcomment="";
		std::string messagetrustchange="";
		std::string trustlisttrustchange="";

		st.ResultText(0,name);
		st.ResultText(1,publickey);
		st.ResultText(2,messagetrust);
		st.ResultText(3,trustlisttrust);
		st.ResultText(4,thisid);
		st.ResultText(5,messagetrustcomment);
		st.ResultText(6,trustlisttrustcomment);
		st.ResultText(7,messagetrustchange);
		st.ResultText(8,trustlisttrustchange);

		content+="<tr>";
		content+="<td><a href=\"peerdetails.htm?identityid="+thisid+"\">"+SanitizeOutput(CreateShortIdentityName(name,publickey))+"</a></td>";
		content+="<td "+GetClassString(messagetrust)+">"+messagetrust+"</td>";
		content+="<td>"+messagetrustchange+"</td>";
		content+="<td>"+SanitizeOutput(messagetrustcomment)+"</td>";
		content+="<td "+GetClassString(trustlisttrust)+">"+trustlisttrust+"</td>";
		content+="<td>"+trustlisttrustchange+"</td>";
		content+="<td>"+SanitizeOutput(trustlisttrustcomment)+"</td>";
		content+="</tr>\r\n";

		st.Step();
	}

	st=m_db->Prepare("SELECT Name,PublicKey,MessageTrust,TrustListTrust,tblIdentity.IdentityID,tblPeerTrust.MessageTrustComment,tblPeerTrust.TrustListTrustComment,MessageTrustChange,TrustListTrustChange FROM tblPeerTrust INNER JOIN tblIdentity ON tblPeerTrust.IdentityID=tblIdentity.IdentityID WHERE tblPeerTrust.TargetIdentityID=? ORDER BY Name COLLATE NOCASE;");
	st.Bind(0,identityid);
	st.Step();

	content+="<tr><th colspan=\"5\"><hr></th></tr>";
	content+="<tr><th colspan=\"5\">";
	content+=m_trans->Get("web.page.peerdetails.trustofthisidentityfromotheridentities");
	content+="</th></tr>";
	content+="<tr><td></td><th>"+m_trans->Get("web.page.peerdetails.messagetrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.lastchange")+"</th><th>"+m_trans->Get("web.page.peerdetails.messagecomment")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlisttrust")+"</th><th>"+m_trans->Get("web.page.peerdetails.lastchange")+"</th><th>"+m_trans->Get("web.page.peerdetails.trustlistcomment")+"</th></tr>";
	while(st.RowReturned())
	{
		std::string thisid="";
		std::string messagetrustcomment="";
		std::string trustlisttrustcomment="";
		std::string messagetrustchange="";
		std::string trustlisttrustchange="";

		st.ResultText(0,name);
		st.ResultText(1,publickey);
		st.ResultText(2,messagetrust);
		st.ResultText(3,trustlisttrust);
		st.ResultText(4,thisid);
		st.ResultText(5,messagetrustcomment);
		st.ResultText(6,trustlisttrustcomment);
		st.ResultText(7,messagetrustchange);
		st.ResultText(8,trustlisttrustchange);

		content+="<tr>";
		content+="<td><a href=\"peerdetails.htm?identityid="+thisid+"\">"+SanitizeOutput(CreateShortIdentityName(name,publickey))+"</a></td>";
		content+="<td "+GetClassString(messagetrust)+">"+messagetrust+"</td>";
		content+="<td>"+messagetrustchange+"</td>";
		content+="<td>"+SanitizeOutput(messagetrustcomment)+"</td>";
		content+="<td "+GetClassString(trustlisttrust)+">"+trustlisttrust+"</td>";
		content+="<td>"+trustlisttrustchange+"</td>";
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
