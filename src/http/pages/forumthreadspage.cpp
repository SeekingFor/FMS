#include "../../../include/http/pages/forumthreadspage.h"
#include "../../../include/stringfunctions.h"
#include <cmath>
#include <algorithm>
/*
const std::string ForumThreadsPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
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
		content+="<td>"+m_trans->Get("web.page.forumthreads.forum")+" <a href=\"forumthreads.htm?boardid="+boardidstr+"\">"+SanitizeOutput(boardname)+"</a></td>";
	}
	content+="<td><a href=\"forumthreads.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"&formaction=markallread\">"+m_trans->Get("web.page.forumthreads.markallread")+"</a></td>";
	content+="<td><a href=\"forumcreatepost.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+m_trans->Get("web.page.forumthreads.newpost")+"</a></td>";
	content+="</tr>";
	content+="</table>\r\n";

	content+="<table class=\"threadinfo\">";
	content+="<thead><tr><th>"+m_trans->Get("web.page.forumthreads.header.new")+"</th><th>"+m_trans->Get("web.page.forumthreads.header.subject")+"</th><th>"+m_trans->Get("web.page.forumthreads.header.startedby")+"</th><th>"+m_trans->Get("web.page.forumthreads.header.replies")+"</th><th>"+m_trans->Get("web.page.forumthreads.header.lastpost")+"</th></tr></thread>\r\n";
	
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
			content+="<img src=\"showfile.htm?file=images/new_posts.png\" title=\""+m_trans->Get("web.page.forum.newposts")+"\">";
		}
		else
		{
			content+="<img src=\"showfile.htm?file=images/no_new_posts.png\" title=\""+m_trans->Get("web.page.forum.nonewposts")+"\">";
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
		content+=lastmessagedate+"<br />"+m_trans->Get("web.page.forumthreads.by")+" <a href=\"peerdetails.htm?identityid="+lastmessageidentityidstr+"\">"+FixFromName(lastmessagefromname)+"</a>";
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
		
		content+="<td class=\"pages\" colspan=\"7\">"+m_trans->Get("web.page.forumthreads.pages")+" ";

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

		content+="<form><input type=\"hidden\" name=\"boardid\" value=\""+boardidstr+"\"><input type=\"text\" name=\"currentpage\"><input type=\"submit\" value=\""+m_trans->Get("web.page.forumthreads.go")+"\"></form>";
		
		content+="</td>";

		content+="</tr>\r\n";
	}
	
	content+="</table>\r\n";

	return content;
}
*/
const std::string ForumTemplateThreadsPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string result("");
	std::string maincontent("");
	std::vector<std::pair<std::string,std::string> > breadcrumblinks;
	std::map<std::string,std::string> vars;
	int boardid=-1;
	std::string boardidstr("-1");
	int page=1;
	std::string pagestr("1");
	int startrow=0;
	std::string startrowstr("0");
	int rowsperpage=25;
	std::string rowsperpagestr("25");
	std::string boardname("");
	int threadcount=0;
	std::string sql("");
	std::string threadrowodd("");
	std::string threadroweven("");
	std::string threadrows("");
	int count=0;
	SQLite3DB::Statement newthreadpostst=m_db->Prepare("SELECT tblMessage.MessageID FROM tblThreadPost INNER JOIN tblMessage ON tblThreadPost.MessageID=tblMessage.MessageID WHERE tblThreadPost.ThreadID=? AND tblMessage.Read=0 LIMIT 0,1;");
	SQLite3DB::Statement replycountst=m_db->Prepare("SELECT IFNULL(COUNT(*)-1,0) FROM tblThreadPost WHERE ThreadID=?;");
	SQLite3DB::Statement boardnamest=m_db->Prepare("SELECT tblBoard.BoardName FROM tblBoard WHERE BoardID=?;");
	SQLite3DB::Statement threadcountst=m_db->Prepare("SELECT COUNT(*) FROM tblThread WHERE BoardID=?;");

	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second;
		StringFunctions::Convert(boardidstr,boardid);
	}
	else
	{
		boardid=m_viewstate.GetBoardID();
	}
	if(queryvars.find("page")!=queryvars.end())
	{
		pagestr=(*queryvars.find("page")).second;
		StringFunctions::Convert(pagestr,page);
		if(page<0)
		{
			page=0;
			pagestr="0";
		}
	}
	else
	{
		page=m_viewstate.GetPage();
	}
	page=(std::max)(page,1);
	StringFunctions::Convert(boardid,boardidstr);
	StringFunctions::Convert(page,pagestr);
	m_viewstate.SetBoardID(boardid);
	m_viewstate.SetPage(page);

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="markallread" && boardid!=-1 && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement markst=m_db->Prepare("UPDATE tblMessage SET Read=1 WHERE tblMessage.Read=0 AND tblMessage.MessageID IN (SELECT MessageID FROM tblThread INNER JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE tblThread.BoardID=?);");
		markst.Bind(0,boardid);
		markst.Step();
	}

	startrow=(page-1)*rowsperpage;
	StringFunctions::Convert(startrow,startrowstr);

	boardnamest.Bind(0,boardid);
	boardnamest.Step();
	if(boardnamest.RowReturned())
	{
		boardnamest.ResultText(0,boardname);
	}

	breadcrumblinks.push_back(std::pair<std::string,std::string>("forummain.htm?viewstate="+m_viewstate.GetViewStateID(),SanitizeOutput(m_trans->Get("web.navlink.browseforums"))));
	breadcrumblinks.push_back(std::pair<std::string,std::string>(m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&page="+pagestr,SanitizeOutput(boardname)));
	CreateBreadcrumbLinks(breadcrumblinks,result);
	vars["LOCATIONBREADCRUMBS"]=result;

	vars["MARKALLREADLINK"]="<a href=\""+m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&currentpage="+pagestr+"&formaction=markallread&"+CreateLinkFormPassword()+"\"><img src=\"images/mail_generic.png\" border=\"0\" style=\"vertical-align:bottom;\">"+m_trans->Get("web.page.forumthreads.markallread")+"</a>";
	vars["NEWPOSTLINK"]="<a href=\"forumcreatepost.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&currentpage="+pagestr+"\"><img src=\"images/mail_new3.png\" border=\"0\" style=\"vertical-align:bottom;\">"+m_trans->Get("web.page.forumthreads.newpost")+"</a>";

	// thread rows

	sql="SELECT tblThread.ThreadID, tblThread.LastMessageID, tblLastMessage.FromName, tblLastMessage.MessageDate || ' ' || tblLastMessage.MessageTime, tblFirstMessage.Subject, tblFirstMessage.FromName, tblFirstMessage.IdentityID, tblLastMessage.IdentityID";
	sql+=" FROM tblThread INNER JOIN tblMessage AS tblLastMessage ON tblThread.LastMessageID=tblLastMessage.MessageID INNER JOIN tblMessage AS tblFirstMessage ON tblThread.FirstMessageID=tblFirstMessage.MessageID";
	sql+=" WHERE tblThread.BoardID=?";
	sql+=" ORDER BY tblLastMessage.MessageDate || tblLastMessage.MessageTime DESC";
	sql+=" LIMIT "+startrowstr+","+rowsperpagestr+";";

	m_templatehandler.GetSection("FORUMTHREADROWODD",threadrowodd);
	m_templatehandler.GetSection("FORUMTHREADROWEVEN",threadroweven);

	SQLite3DB::Statement threadst=m_db->Prepare(sql);
	threadst.Bind(0,boardid);
	threadst.Step();
	while(threadst.RowReturned() && count++<rowsperpage)
	{
		std::map<std::string,std::string> rowvars;
		std::string thisrow("");
		std::string threadidstr="";
		std::string lastmessageidstr="";
		std::string lastmessagefromname="";
		std::string lastmessagedate="";
		std::string firstmessagesubject="";
		std::string firstmessagefromname="";
		std::string firstmessageidentityidstr="";
		std::string lastmessageidentityidstr="";
		std::string threadlastpostinfo("");
		std::map<std::string,std::string> lastpostvars;

		threadst.ResultText(0,threadidstr);
		threadst.ResultText(1,lastmessageidstr);
		threadst.ResultText(2,lastmessagefromname);
		threadst.ResultText(3,lastmessagedate);
		threadst.ResultText(4,firstmessagesubject);
		threadst.ResultText(5,firstmessagefromname);
		threadst.ResultText(6,firstmessageidentityidstr);
		threadst.ResultText(7,lastmessageidentityidstr);

		newthreadpostst.Bind(0,threadidstr);
		newthreadpostst.Step();
		if(newthreadpostst.RowReturned())
		{
			rowvars["NEWPOSTIMAGE"]="<img src=\"images/new_posts.png\" title=\""+m_trans->Get("web.page.forum.newposts")+"\">";
		}
		else
		{
			rowvars["NEWPOSTIMAGE"]="<img src=\"images/no_new_posts.png\" title=\""+m_trans->Get("web.page.forum.nonewposts")+"\">";
		}
		newthreadpostst.Reset();

		rowvars["THREADSUBJECT"]="<a href=\"forumviewthread.htm?viewstate="+m_viewstate.GetViewStateID()+"&threadid="+threadidstr+"&page="+pagestr+"&boardid="+boardidstr+"\">"+SanitizeOutput(firstmessagesubject)+"</a>";
		rowvars["STARTEDBY"]="<a href=\"peerdetails.htm?identityid="+firstmessageidentityidstr+"\">"+FixAuthorName(firstmessagefromname)+"</a>";
		
		replycountst.Bind(0,threadidstr);
		replycountst.Step();
		if(replycountst.RowReturned())
		{
			replycountst.ResultText(0,rowvars["REPLIES"]);
		}
		else
		{
			rowvars["REPLIES"]="0";
		}
		replycountst.Reset();

		rowvars["THREADLASTPOSTDATE"]=lastmessagedate;
		rowvars["THREADLASTPOSTAUTHOR"]="<a href=\"peerdetails.htm?identityid="+lastmessageidentityidstr+"\">"+FixAuthorName(lastmessagefromname)+"</a>";

		if(count%2==1)
		{
			m_templatehandler.PerformReplacements(threadrowodd,rowvars,thisrow);
		}
		else
		{
			m_templatehandler.PerformReplacements(threadroweven,rowvars,thisrow);
		}
		threadrows+=thisrow;

		threadst.Step();
	}

	vars["FORUMTHREADROWS"]=threadrows;

	// thread page numbering
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
		std::string pagenumbers("");

		for(int i=1; i<=totalpages; i++)
		{
			if(i==1 || (i>page-3 && i<page+3) || i==totalpages)
			{
				std::string pagestr="";
				StringFunctions::Convert(i,pagestr);
				if(lastwrote!=i-1)
				{
					pagenumbers+="&nbsp;...";
				}
				if(i!=page)
				{
					pagenumbers+="&nbsp;<a href=\""+m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&page="+pagestr+"\">"+pagestr+"</a>";
				}
				else
				{
					pagenumbers+="&nbsp;"+pagestr;
				}
				lastwrote=i;
			}
		}

		m_templatehandler.GetSection("FORUMTHREADPAGES",vars["FORUMTHREADPAGES"]);
		vars["PAGENUMBERS"]=pagenumbers;
		vars["PAGENUMBERFORM"]="<form><input type=\"hidden\" name=\"boardid\" value=\""+boardidstr+"\"><input type=\"hidden\" name=\"viewstate\" value=\""+m_viewstate.GetViewStateID()+"\"><input class=\"pagetext\" type=\"text\" name=\"page\"><input type=\"submit\" value=\""+m_trans->Get("web.page.forumthreads.go")+"\"></form>";

	}
	else
	{
		vars["FORUMTHREADPAGES"]="";
		vars["PAGENUMBERS"]="";
		vars["PAGENUMBERFORM"]="";
	}

	std::vector<std::string> ignored;
	ignored.push_back("FORUMTHREADPAGES");
	m_templatehandler.GetSection("FORUMTHREADSCONTENT",maincontent,ignored);
	m_templatehandler.PerformReplacements(maincontent,vars,result);

	return result;
}
