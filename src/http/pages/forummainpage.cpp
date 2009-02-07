#include "../../../include/http/pages/forummainpage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ForumMainPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";

	content+=CreateForumHeader();

	content+="<table class=\"foruminfo\">\r\n";
	content+="<thead><tr><th>New</th><th>Forum</th><th>Posts</th><th>Last Post</th></tr></thead>\r\n";

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
			content+="<img src=\"showimage.htm?image=images/new_posts.png\" title=\"New Posts\">";
		}
		else
		{
			content+="<img src=\"showimage.htm?image=images/no_new_posts.png\" title=\"No New Posts\">";
		}
		newmessagesst.Reset();

		content+="</td>";

		content+="<td class=\"forumname\">";
		content+="<a href=\"forumthreads.htm?boardid="+boardidstr+"\">"+SanitizeOutput(boardname)+"</a><br />";
		content+="<span class=\"description\">"+SanitizeOutput(boarddescription)+"</span>";
		content+="</td>";
		content+="<td class=\"postcount\">";
		content+=postcountstr+" posts";
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

			content+="Last post on "+messagedate+" in<br />";
			content+="<a href=\"forumviewthread.htm?threadid="+threadidstr+"&boardid="+boardidstr+"#"+messageidstr+"\">"+FixSubject(subject)+"</a> by <a href=\"peerdetails.htm?identityid="+identityidstr+"\" title=\""+SanitizeOutput(fromname)+"\">"+FixFromName(fromname)+"</a>";
		}
		content+="</td>";
		lastmessagest.Reset();

		content+="</tr>\r\n";
		st.Step();
	}

	content+="</table>\r\n";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}
