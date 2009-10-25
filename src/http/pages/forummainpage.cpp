#include "../../../include/http/pages/forummainpage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif
/*
const std::string ForumMainPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";

	content+=CreateForumHeader();

	content+="<table class=\"foruminfo\">\r\n";
	content+="<thead><tr><th>"+m_trans->Get("web.page.forummain.header.new")+"</th><th>"+m_trans->Get("web.page.forummain.header.forum")+"</th><th>"+m_trans->Get("web.page.forummain.header.posts")+"</th><th>"+m_trans->Get("web.page.forummain.header.lastpost")+"</th></tr></thead>\r\n";

	SQLite3DB::Statement newmessagesst=m_db->Prepare("SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID WHERE tblMessageBoard.BoardID=? AND tblThread.BoardID=? AND tblMessage.Read=0 LIMIT 0,1;");
	SQLite3DB::Statement lastmessagest=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.IdentityID, tblMessage.FromName, tblMessage.Subject, tblMessage.MessageDate || ' ' || tblMessage.MessageTime, tblThread.ThreadID FROM tblMessage INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID WHERE tblThread.BoardID=? ORDER BY tblMessage.MessageDate || tblMessage.MessageTime DESC LIMIT 0,1;");
	
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblBoard.BoardID, BoardName, BoardDescription, COUNT(tblThreadPost.MessageID) FROM tblBoard LEFT JOIN tblThread ON tblBoard.BoardID=tblThread.BoardID LEFT JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE Forum='true' GROUP BY tblBoard.BoardID ORDER BY BoardName COLLATE NOCASE;");
	st.Step();
	while(st.RowReturned())
	{
		int boardid=-1;
		std::string boardidstr="-1";
		std::string boardname="";
		std::string boarddescription="";
		std::string postcountstr="";

		st.ResultInt(0,boardid);
		st.ResultText(0,boardidstr);
		st.ResultText(1,boardname);
		st.ResultText(2,boarddescription);
		st.ResultText(3,postcountstr);

		content+="<tr>";
		content+="<td class=\"newposts\">";

		newmessagesst.Bind(0,boardid);
		newmessagesst.Bind(1,boardid);
		newmessagesst.Step();
		if(newmessagesst.RowReturned())
		{
			content+="<img src=\"showfile.htm?file=images/new_posts.png\" title=\""+m_trans->Get("web.page.forum.newposts")+"\">";
		}
		else
		{
			content+="<img src=\"showfile.htm?file=images/no_new_posts.png\" title=\""+m_trans->Get("web.page.forum.nonewposts")+"\">";
		}
		newmessagesst.Reset();

		content+="</td>";

		content+="<td class=\"forumname\">";
		content+="<a href=\"forumthreads.htm?boardid="+boardidstr+"\">"+SanitizeOutput(boardname)+"</a><br />";
		content+="<span class=\"description\">"+SanitizeOutput(boarddescription)+"</span>";
		content+="</td>";
		content+="<td class=\"postcount\">";
		content+=postcountstr+" "+m_trans->Get("web.page.forummain.posts");
		content+="</td>";

		lastmessagest.Bind(0,boardid);
		lastmessagest.Step();
		content+="<td class=\"lastpost\">";
		if(lastmessagest.RowReturned())
		{
			std::string messageidstr="";
			std::string identityidstr="";
			std::string fromname="";
			std::string messagedate="";
			std::string subject="";
			std::string threadidstr="";

			lastmessagest.ResultText(0,messageidstr);
			lastmessagest.ResultText(1,identityidstr);
			lastmessagest.ResultText(2,fromname);
			lastmessagest.ResultText(3,subject);
			lastmessagest.ResultText(4,messagedate);
			lastmessagest.ResultText(5,threadidstr);

			content+=m_trans->Get("web.page.forummain.lastposton")+" "+messagedate+" "+m_trans->Get("web.page.forummain.in")+"<br />";
			content+="<a href=\"forumviewthread.htm?threadid="+threadidstr+"&boardid="+boardidstr+"#"+messageidstr+"\">"+FixSubject(subject)+"</a> "+m_trans->Get("web.page.forummain.by")+" <a href=\"peerdetails.htm?identityid="+identityidstr+"\" title=\""+SanitizeOutput(fromname)+"\">"+FixFromName(fromname)+"</a>";
		}
		content+="</td>";
		lastmessagest.Reset();

		content+="</tr>\r\n";
		st.Step();
	}

	content+="</table>\r\n";

	return content;
}
*/
const std::string ForumTemplateMainPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string result("");
	std::string maincontent("");
	std::string forumrow("");
	std::string forumrows("");
	std::vector<std::pair<std::string,std::string> > breadcrumblinks;
	std::map<std::string,std::string> vars;

	m_viewstate.UnsetBoardID();
	m_viewstate.UnsetPage();
	m_viewstate.UnsetThreadID();
	m_viewstate.UnsetMessageID();
	m_viewstate.UnsetReplyToMessageID();

	m_templatehandler.GetSection("FORUMMAINCONTENT",maincontent);
	m_templatehandler.GetSection("FORUMROW",forumrow);

	breadcrumblinks.push_back(std::pair<std::string,std::string>("forummain.htm?viewstate="+m_viewstate.GetViewStateID(),SanitizeOutput(m_trans->Get("web.navlink.browseforums"))));
	CreateBreadcrumbLinks(breadcrumblinks,result);
	vars["LOCATIONBREADCRUMBS"]=result;

	SQLite3DB::Statement newst=m_db->Prepare("SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID WHERE tblMessageBoard.BoardID=? AND tblThread.BoardID=? AND tblMessage.Read=0 LIMIT 0,1;");
	SQLite3DB::Statement lastmessagest=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.IdentityID, tblMessage.FromName, tblMessage.Subject, tblMessage.MessageDate || ' ' || tblMessage.MessageTime, tblThread.ThreadID FROM tblMessage INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID WHERE tblThread.BoardID=? ORDER BY tblMessage.MessageDate || tblMessage.MessageTime DESC LIMIT 0,1;");
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblBoard.BoardID, BoardName, BoardDescription, MessageCount FROM tblBoard WHERE Forum='true' ORDER BY BoardName COLLATE NOCASE;");
	st.Step();
	while(st.RowReturned())
	{
		std::string thisrow("");
		std::map<std::string,std::string> rowvars;
		std::string boardidstr("");
		std::string boardname("");
		std::string boarddescription("");
		std::string postcountstr("");

		st.ResultText(0,boardidstr);
		st.ResultText(1,boardname);
		st.ResultText(2,boarddescription);
		st.ResultText(3,postcountstr);

		newst.Bind(0,boardidstr);
		newst.Bind(1,boardidstr);
		newst.Step();
		if(newst.RowReturned())
		{
			rowvars["NEWPOSTIMAGE"]="<img src=\"images/new_posts.png\" title=\""+m_trans->Get("web.page.forum.newposts")+"\">";
		}
		else
		{
			rowvars["NEWPOSTIMAGE"]="<img src=\"images/no_new_posts.png\" title=\""+m_trans->Get("web.page.forum.nonewposts")+"\">";
		}
		newst.Reset();

		lastmessagest.Bind(0,boardidstr);
		lastmessagest.Step();
		if(lastmessagest.RowReturned())
		{
			std::string lastpost="";
			std::string messageidstr="";
			std::string identityidstr="";
			std::string fromname="";
			std::string messagedate="";
			std::string subject="";
			std::string threadidstr="";
			std::map<std::string,std::string> lastpostvars;

			m_templatehandler.GetSection("FORUMLASTPOST",lastpost);

			lastmessagest.ResultText(0,messageidstr);
			lastmessagest.ResultText(1,identityidstr);
			lastmessagest.ResultText(2,fromname);
			lastmessagest.ResultText(3,subject);
			lastmessagest.ResultText(4,messagedate);
			lastmessagest.ResultText(5,threadidstr);

			lastpostvars["FORUMLASTPOSTDATE"]=messagedate;
			lastpostvars["FORUMLASTPOSTSUBJECT"]="<a href=\"forumviewthread.htm?viewstate="+m_viewstate.GetViewStateID()+"&threadid="+threadidstr+"&boardid="+boardidstr+"#"+messageidstr+"\">"+FixSubject(subject)+"</a>";
			lastpostvars["FORUMLASTPOSTAUTHOR"]="<a href=\"peerdetails.htm?identityid="+identityidstr+"\" title=\""+SanitizeOutput(fromname)+"\">"+FixAuthorName(fromname)+"</a>";

			m_templatehandler.PerformReplacements(lastpost,lastpostvars,rowvars["FORUMLASTPOSTINFO"]);
		}
		else
		{
			rowvars["FORUMLASTPOSTINFO"]="";
		}
		lastmessagest.Reset();

		rowvars["FORUMNAME"]="<a href=\"forumthreads.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"\">"+SanitizeOutput(boardname)+"</a><br />";
		rowvars["FORUMDESCRIPTION"]=SanitizeOutput(boarddescription);
		rowvars["FORUMPOSTCOUNT"]=postcountstr;

		m_templatehandler.PerformReplacements(forumrow,rowvars,thisrow);
		forumrows+=thisrow;

		st.Step();
	}

	vars["FORUMROWS"]=forumrows;

	m_templatehandler.PerformReplacements(maincontent,vars,result);

	return result;
}
