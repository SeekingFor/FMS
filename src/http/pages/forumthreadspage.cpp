#include "../../../include/http/pages/forumthreadspage.h"
#include "../../../include/stringfunctions.h"
#include <cmath>

const std::string ForumThreadsPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	int currentpage=1;
	std::string currentpagestr="1";
	std::string content="";
	int startrow=0;
	std::string startrowstr="0";
	int rowsperpage=25;
	std::string rowsperpagestr="25";
	std::string sql="";
	int boardid=-1;
	std::string boardidstr="-1";
	int count=0;
	int threadcount=0;
	SQLite3DB::Statement newthreadpostst=m_db->Prepare("SELECT tblMessage.MessageID FROM tblThreadPost INNER JOIN tblMessage ON tblThreadPost.MessageID=tblMessage.MessageID WHERE tblThreadPost.ThreadID=? AND tblMessage.Read=0 LIMIT 0,1;");
	SQLite3DB::Statement replycountst=m_db->Prepare("SELECT COUNT(*)-1 FROM tblThreadPost WHERE ThreadID=?;");
	SQLite3DB::Statement boardnamest=m_db->Prepare("SELECT tblBoard.BoardName FROM tblBoard WHERE BoardID=?;");
	SQLite3DB::Statement threadcountst=m_db->Prepare("SELECT COUNT(*) FROM tblThread WHERE BoardID=?;");

	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second;
		StringFunctions::Convert(boardidstr,boardid);
	}
	if(queryvars.find("currentpage")!=queryvars.end())
	{
		currentpagestr=(*queryvars.find("currentpage")).second;
		StringFunctions::Convert(currentpagestr,currentpage);
		if(currentpage<0)
		{
			currentpage=0;
		}
		StringFunctions::Convert(currentpage,currentpagestr);
	}
	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="markallread" && boardid!=-1)
	{
		SQLite3DB::Statement markst=m_db->Prepare("UPDATE tblMessage SET Read=1 WHERE tblMessage.Read=0 AND tblMessage.MessageID IN (SELECT MessageID FROM tblThread INNER JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE tblThread.BoardID=?);");
		markst.Bind(0,boardid);
		markst.Step();
	}

	startrow=(currentpage-1)*rowsperpage;
	StringFunctions::Convert(startrow,startrowstr);

	content+=CreateForumHeader();

	content+="<table class=\"forumheader\">";
	content+="<tr>";

	boardnamest.Bind(0,boardid);
	boardnamest.Step();
	if(boardnamest.RowReturned())
	{
		std::string boardname="";
		boardnamest.ResultText(0,boardname);
		content+="<td>Forum : <a href=\"forumthreads.htm?boardid="+boardidstr+"\">"+SanitizeOutput(boardname)+"</a></td>";
	}
	content+="<td><a href=\"forumthreads.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"&formaction=markallread\">Mark All Read</a></td>";
	content+="<td><a href=\"forumcreatepost.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"\">New Post</a></td>";
	content+="</tr>";
	content+="</table>\r\n";

	content+="<table class=\"threadinfo\">";
	content+="<thead><tr><th>New</th><th>Subject</th><th>Started By</th><th>Replies</th><th>Last Post</th></tr></thread>\r\n";
	
	sql="SELECT tblThread.ThreadID, tblThread.LastMessageID, tblLastMessage.FromName, tblLastMessage.MessageDate || ' ' || tblLastMessage.MessageTime, tblFirstMessage.Subject, tblFirstMessage.FromName, tblFirstMessage.IdentityID, tblLastMessage.IdentityID";
	sql+=" FROM tblThread INNER JOIN tblMessage AS tblLastMessage ON tblThread.LastMessageID=tblLastMessage.MessageID INNER JOIN tblMessage AS tblFirstMessage ON tblThread.FirstMessageID=tblFirstMessage.MessageID";
	sql+=" WHERE tblThread.BoardID=?";
	sql+=" ORDER BY tblLastMessage.MessageDate || tblLastMessage.MessageTime DESC";
	sql+=" LIMIT "+startrowstr+","+rowsperpagestr+";";

	SQLite3DB::Statement threadst=m_db->Prepare(sql);
	threadst.Bind(0,boardid);
	threadst.Step();
	count=0;
	while(threadst.RowReturned() && count++<rowsperpage)
	{
		std::string threadidstr="";
		std::string lastmessageidstr="";
		std::string lastmessagefromname="";
		std::string lastmessagedate="";
		std::string firstmessagesubject="";
		std::string firstmessagefromname="";
		std::string firstmessageidentityidstr="";
		std::string lastmessageidentityidstr="";

		threadst.ResultText(0,threadidstr);
		threadst.ResultText(1,lastmessageidstr);
		threadst.ResultText(2,lastmessagefromname);
		threadst.ResultText(3,lastmessagedate);
		threadst.ResultText(4,firstmessagesubject);
		threadst.ResultText(5,firstmessagefromname);
		threadst.ResultText(6,firstmessageidentityidstr);
		threadst.ResultText(7,lastmessageidentityidstr);

		content+="<tr>";
		content+="<td class=\"newposts\">";

		newthreadpostst.Bind(0,threadidstr);
		newthreadpostst.Step();
		if(newthreadpostst.RowReturned())
		{
			content+="<img src=\"showimage.htm?image=images/new_posts.png\" title=\"New Posts\">";
		}
		else
		{
			content+="<img src=\"showimage.htm?image=images/no_new_posts.png\" title=\"No New Posts\">";
		}
		newthreadpostst.Reset();

		content+="</td>";
		content+="<td class=\"threadsubject\">";
		content+="<a href=\"forumviewthread.htm?threadid="+threadidstr+"&currentpage="+currentpagestr+"&boardid="+boardidstr+"\">"+SanitizeOutput(firstmessagesubject)+"</a>";
		content+="</td>";
		content+="<td class=\"threadauthor\">";
		content+="<a href=\"peerdetails.htm?identityid="+firstmessageidentityidstr+"\">"+FixFromName(firstmessagefromname)+"</a>";
		content+="</td>";

		content+="<td class=\"threadreplies\">";

		replycountst.Bind(0,threadidstr);
		replycountst.Step();
		if(replycountst.RowReturned())
		{
			std::string count="0";
			replycountst.ResultText(0,count);
			content+=count;
		}
		else
		{
			content+="0";
		}
		replycountst.Reset();

		content+="</td>";

		content+="<td class=\"threadlastpost\">";
		content+=lastmessagedate+"<br />by <a href=\"peerdetails.htm?identityid="+lastmessageidentityidstr+"\">"+FixFromName(lastmessagefromname)+"</a>";
		content+="</td>";

		content+="</tr>\r\n";

		threadst.Step();
	}

	threadcountst.Bind(0,boardid);
	threadcountst.Step();
	if(threadcountst.RowReturned())
	{
		threadcountst.ResultInt(0,threadcount);
	}

	if(threadcount>=rowsperpage)
	{
		int totalpages=ceil(static_cast<float>(threadcount)/static_cast<float>(rowsperpage));
		int lastwrote=0;
		content+="<tr>";
		
		content+="<td class=\"pages\" colspan=\"7\">Pages : ";

		for(int i=1; i<=totalpages; i++)
		{
			if(i==1 || (i>currentpage-3 && i<currentpage+3) || i==totalpages)
			{
				std::string pagestr="";
				StringFunctions::Convert(i,pagestr);
				if(lastwrote!=i-1)
				{
					content+="&nbsp;...";
				}
				if(i!=currentpage)
				{
					content+="&nbsp;<a href=\"forumthreads.htm?boardid="+boardidstr+"&currentpage="+pagestr+"\">"+pagestr+"</a>";
				}
				else
				{
					content+="&nbsp;"+pagestr;
				}
				lastwrote=i;
			}
		}

		content+="<form><input type=\"hidden\" name=\"boardid\" value=\""+boardidstr+"\"><input type=\"text\" name=\"currentpage\"><input type=\"submit\" value=\"Go\"></form>";
		
		content+="</td>";

		content+="</tr>\r\n";
	}
	
	content+="</table>\r\n";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}
