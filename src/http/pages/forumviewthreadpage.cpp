#include "../../../include/http/pages/forumviewthreadpage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/unicode/unicodeformatter.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ForumViewThreadPage::FixBody(const std::string &body)
{
	std::string output=body;

	output=StringFunctions::Replace(output,"\r\n","\n");

	UnicodeFormatter::LineWrap(output,80,"",output);

	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");
	output=StringFunctions::Replace(output,"\n","<br />");
	return output;
}

const std::string ForumViewThreadPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";
	std::string threadidstr="";
	std::string boardidstr="";
	std::string currentpagestr="";
	std::string boardname="";
	std::string firstunreadidstr="";

	if(queryvars.find("threadid")!=queryvars.end())
	{
		threadidstr=(*queryvars.find("threadid")).second;
	}
	if(queryvars.find("currentpage")!=queryvars.end())
	{
		currentpagestr=(*queryvars.find("currentpage")).second;
	}
	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second;
	}

	content+=CreateForumHeader();

	SQLite3DB::Statement firstunreadst=m_db->Prepare("SELECT tblMessage.MessageID FROM tblThreadPost INNER JOIN tblMessage ON tblThreadPost.MessageID=tblMessage.MessageID WHERE ThreadID=? AND tblMessage.Read=0;");
	firstunreadst.Bind(0,threadidstr);
	firstunreadst.Step();
	if(firstunreadst.RowReturned())
	{
		firstunreadst.ResultText(0,firstunreadidstr);
	}

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="markunread")
	{
		SQLite3DB::Statement updateread=m_db->Prepare("UPDATE tblMessage SET Read=0 WHERE tblMessage.MessageID IN (SELECT MessageID FROM tblThreadPost WHERE ThreadID=?);");
		updateread.Bind(0,threadidstr);
		updateread.Step();
	}
	else
	{
		SQLite3DB::Statement updateread=m_db->Prepare("UPDATE tblMessage SET Read=1 WHERE tblMessage.MessageID IN (SELECT MessageID FROM tblThreadPost WHERE ThreadID=?);");
		updateread.Bind(0,threadidstr);
		updateread.Step();
	}

	SQLite3DB::Statement trustst=m_db->Prepare("SELECT LocalMessageTrust, LocalTrustListTrust, PeerMessageTrust, PeerTrustListTrust, Name FROM tblIdentity WHERE IdentityID=?;");

	SQLite3DB::Statement boardnamest=m_db->Prepare("SELECT tblBoard.BoardName FROM tblBoard INNER JOIN tblThread ON tblBoard.BoardID=tblThread.BoardID WHERE tblThread.ThreadID=?;");
	boardnamest.Bind(0,threadidstr);
	boardnamest.Step();

	if(boardnamest.RowReturned())
	{
		boardnamest.ResultText(0,boardname);
	}

	content+="<table class=\"forumheader\">";
	content+="<tr>";
	content+="<td> "+m_trans->Get("web.page.forumviewthread.forum")+" <a href=\"forumthreads.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+SanitizeOutput(boardname)+"</a></td>";
	if(firstunreadidstr!="")
	{
		content+="<td>";
		content+="<a href=\"#"+firstunreadidstr+"\">"+m_trans->Get("web.page.forumviewthread.firstunread")+"</a>";
		content+="</td>";
	}
	content+="<td>";
	content+="<a href=\""+m_pagename+"?formaction=markunread&threadid="+threadidstr+"&boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+m_trans->Get("web.page.forumviewthread.markunread")+"</a>";
	content+="</td>";
	content+="</tr>";
	content+="</table>\r\n";

	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.IdentityID, tblMessage.FromName, tblMessage.Subject, tblMessage.MessageDate || ' ' || tblMessage.MessageTime, tblMessage.Body FROM tblMessage INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID WHERE tblThreadPost.ThreadID=? ORDER BY tblThreadPost.PostOrder;");
	st.Bind(0,threadidstr);

	content+="<table class=\"thread\">";
	st.Step();
	while(st.RowReturned())
	{
		std::string messageidstr="";
		std::string identityidstr="";
		std::string fromname="";
		std::string subject="";
		std::string datetime="";
		std::string body="";
		
		st.ResultText(0,messageidstr);
		st.ResultText(1,identityidstr);
		st.ResultText(2,fromname);
		st.ResultText(3,subject);
		st.ResultText(4,datetime);
		st.ResultText(5,body);

		content+="<tr>";
		content+="<td rowspan=\"2\" class=\"from\">";
		content+="<a name=\""+messageidstr+"\"></a>";
		content+="<a href=\"peerdetails.htm?identityid="+identityidstr+"\">"+FixFromName(fromname)+"</a><br />";

		trustst.Bind(0,identityidstr);
		trustst.Step();
		if(trustst.RowReturned())
		{
			std::string localmessagetrust="";
			std::string localtrustlisttrust="";
			std::string peermessagetrust="";
			std::string peertrustlisttrust="";
			std::string name="";

			trustst.ResultText(0,localmessagetrust);
			trustst.ResultText(1,localtrustlisttrust);
			trustst.ResultText(2,peermessagetrust);
			trustst.ResultText(3,peertrustlisttrust);
			trustst.ResultText(4,name);

			content+="<table class=\"trust\">";
			content+="<tr>";
			content+="<td colspan=\"3\" style=\"text-align:center;\"><a href=\"peertrust.htm?namesearch="+StringFunctions::UriEncode(name)+"\">"+m_trans->Get("web.page.forumviewthread.trust")+"</a></td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td></td><td>"+m_trans->Get("web.page.forumviewthread.local")+"</td><td>"+m_trans->Get("web.page.forumviewthread.peer")+"</td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td>"+m_trans->Get("web.page.forumviewthread.message")+"</td><td>"+localmessagetrust+"</td><td>"+peermessagetrust+"</td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td>"+m_trans->Get("web.page.forumviewthread.trustlist")+"</td><td>"+localtrustlisttrust+"</td><td>"+peertrustlisttrust+"</td>";
			content+="</tr>";
			content+="</table>";
		}

		content+="</td>";
		content+="<td class=\"subject\">";
		content+=SanitizeOutput(subject)+" "+m_trans->Get("web.page.forumviewthread.on")+" "+datetime;
		content+="</td>";
		content+="<td><a href=\"forumcreatepost.htm?replytomessageid="+messageidstr+"&threadid="+threadidstr+"&boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+m_trans->Get("web.page.forumviewthread.reply")+"</a></td>";
		content+="</tr>\r\n";
		content+="<tr>";
		content+="<td class=\"body\" colspan=\"2\">";
		content+=FixBody(body);
		content+="</td>";
		content+="</tr>";
		trustst.Reset();

		st.Step();
	}
	content+="</table>";

	return content;
}
