#include "../../../include/http/pages/forumviewthreadpage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ForumViewThreadPage::FixBody(const std::string &body)
{
	static std::string whitespace=" \t\r\n";
	std::string output=body;

	output=StringFunctions::Replace(output,"\r\n","\n");

	// put \n after 80 contiguous characters in the body
	std::string::size_type prevpos=0;
	std::string::size_type pos=output.find_first_of(whitespace);
	while(pos!=std::string::npos)
	{
		while(pos-prevpos>80)
		{
			output.insert(prevpos+80,"\n");
			prevpos+=81;	// 81 because of the extra newline we just inserted
		}
		prevpos=pos;
		pos=output.find_first_of(whitespace,pos+1);
	}
	while(output.size()-prevpos>80)	// check the last line of the message (no whitespace after it)
	{
		output.insert(prevpos+80,"\n");
		prevpos+=81;
	}

	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");
	output=StringFunctions::Replace(output,"\n","<br />");
	return output;
}

const std::string ForumViewThreadPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
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

	SQLite3DB::Statement updateread=m_db->Prepare("UPDATE tblMessage SET Read=1 WHERE tblMessage.MessageID IN (SELECT MessageID FROM tblThreadPost WHERE ThreadID=?);");
	updateread.Bind(0,threadidstr);
	updateread.Step();

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
	content+="<td> Forum : <a href=\"forumthreads.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+SanitizeOutput(boardname)+"</a></td>";
	if(firstunreadidstr!="")
	{
		content+="<td>";
		content+="<a href=\"#"+firstunreadidstr+"\">First Unread Message</a>";
		content+="</td>";
	}
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
			content+="<td colspan=\"3\" style=\"text-align:center;\"><a href=\"peertrust.htm?namesearch="+StringFunctions::UriEncode(name)+"\">Trust</a></td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td></td><td>Local</td><td>Peer</td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td>Message</td><td>"+localmessagetrust+"</td><td>"+peermessagetrust+"</td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td>Trust List</td><td>"+localtrustlisttrust+"</td><td>"+peertrustlisttrust+"</td>";
			content+="</tr>";
			content+="</table>";
		}

		content+="</td>";
		content+="<td class=\"subject\">";
		content+=SanitizeOutput(subject)+" on "+datetime;
		content+="</td>";
		content+="<td><a href=\"forumcreatepost.htm?replytomessageid="+messageidstr+"&threadid="+threadidstr+"&boardid="+boardidstr+"&currentpage="+currentpagestr+"\">Reply</a></td>";
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

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}
