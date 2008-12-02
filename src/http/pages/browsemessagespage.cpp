#include "../../../include/http/pages/browsemessagespage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/global.h"
#include "../../../include/messagethread.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string BrowseMessagesPage::BuildQueryString(const long startrow, const std::string &boardidstr, const std::string &messageidstr)
{
	std::string returnval="";
	std::string tempval="";

	if(startrow>=0)
	{
		StringFunctions::Convert(startrow,tempval);
		returnval+="startrow="+tempval;
	}

	if(boardidstr!="")
	{
		if(returnval!="")
		{
			returnval+="&";
		}
		returnval+="boardid="+boardidstr;
	}

	if(messageidstr!="")
	{
		if(returnval!="")
		{
			returnval+="&";
		}	
		returnval+="messageid="+messageidstr;
	}

	return returnval;

}

const std::string BrowseMessagesPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";
	long boardid=-1;
	std::string boardidstr="";
	std::string messageidstr="";
	int startrow=0;
	std::string startrowstr="0";
	int rowsperpage=50;
	std::string rowsperpagestr="51";	// one more than rowsperpage so we can know if there are more messages
	std::string sql="";
	int rowcount=0;
	int page=0;

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
	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second;
		StringFunctions::Convert(boardidstr,boardid);
	}
	if(queryvars.find("messageid")!=queryvars.end())
	{
		messageidstr=(*queryvars.find("messageid")).second;
		page=1;
	}

	if(page==0)
	{
		sql="SELECT Subject, FromName, MessageDate || ' ' || MessageTime, Body, tblMessage.MessageID ";
		sql+="FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID ";
		if(boardidstr!="")
		{
			sql+="WHERE tblMessageBoard.BoardID=? ";
		}
		sql+="ORDER BY MessageDate || ' ' || MessageTime DESC ";
		sql+="LIMIT "+startrowstr+","+rowsperpagestr+";";

		SQLite3DB::Statement st=m_db->Prepare(sql);
		if(boardidstr!="")
		{
			st.Bind(0,boardidstr);
		}
		st.Step();

		content+="<table class=\"small90\">";
		content+="<tr><th>Subject</th><th>From</th><th>Date</th></tr>";

		rowcount=0;
		while(st.RowReturned() && rowcount++<rowsperpage)
		{
			std::string subject="";
			std::string fromname="";
			std::string messagedate="";
			std::string body="";
			std::string messageidstr="";

			st.ResultText(0,subject);
			st.ResultText(1,fromname);
			st.ResultText(2,messagedate);
			st.ResultText(3,body);
			st.ResultText(4,messageidstr);

			if(body.size()>400)
			{
				body.erase(400);
				body+="...";
			}

			content+="<tr>";
			content+="<td title=\""+StringFunctions::Replace(SanitizeOutput(body),"\n","\r\n")+"\"><a href=\""+m_pagename+"?"+BuildQueryString(startrow,boardidstr,messageidstr)+"\">"+SanitizeOutput(subject)+"</a></td>";
			content+="<td>"+fromname+"</td>";
			content+="<td>"+SanitizeOutput(messagedate)+"</td>";
			content+="</tr>\r\n";

			st.Step();
		}
		
		if(startrow>0 || st.RowReturned())
		{
			content+="<tr>";

			if(startrow>0)
			{
				int thisstartrow=startrow-rowsperpage;
				if(thisstartrow<0)
				{
					thisstartrow=0;
				}
				content+="<td><a href=\""+m_pagename+"?"+BuildQueryString(thisstartrow,boardidstr,"")+"\">&lt;--</a></td>";
			}
			else
			{
				content+="<td></td>";
			}

			content+="<td></td>";

			if(st.RowReturned())
			{
				int thisstartrow=startrow+rowsperpage;
				content+="<td><a href=\""+m_pagename+"?"+BuildQueryString(thisstartrow,boardidstr,"")+"\">--&gt;</a></td>";
			}
			content+="</tr>";
		}

		content+="</table>";
	}
	else if(page==1)
	{
		sql="SELECT Body, FromName, MessageDate || ' ' || MessageTime, Subject FROM tblMessage WHERE MessageID=?;";
		SQLite3DB::Statement st=m_db->Prepare(sql);
		st.Bind(0,messageidstr);

		st.Step();
		if(st.RowReturned())
		{
			std::string body="";
			std::string fromname="";
			std::string messagedate="";
			std::string subject="";
			std::string boards="";

			st.ResultText(0,body);
			st.ResultText(1,fromname);
			st.ResultText(2,messagedate);
			st.ResultText(3,subject);

			// get boards message was posted to
			SQLite3DB::Statement st2=m_db->Prepare("SELECT tblBoard.BoardID, tblBoard.BoardName FROM tblBoard INNER JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblMessageBoard.MessageID=?;");
			st2.Bind(0,messageidstr);
			st2.Step();
			while(st2.RowReturned())
			{
				std::string boardname="";
				std::string innerboardidstr="";

				st2.ResultText(0,innerboardidstr);
				st2.ResultText(1,boardname);
				
				if(boards!="")
				{
					boards+=",&nbsp;";
				}

				boards+="<a href=\""+m_pagename+"?"+BuildQueryString(0,innerboardidstr,"")+"\">"+SanitizeOutput(boardname)+"</a>";

				st2.Step();
			}

			content+="<div class=\"post\">";
			content+="<div class=\"postboards\">";
			content+=boards;
			content+="</div>";
			content+="<div class=\"postsubject\">";
			content+=SanitizeOutput(subject);
			content+="</div>";
			content+="<div class=\"postfrom\">";
			content+=SanitizeOutput(fromname);
			content+="</div>";
			content+="<div class=\"postdate\">";
			content+=SanitizeOutput(messagedate);
			content+="</div>";
			content+="<div class=\"postbody\">";
			content+=SanitizeOutput(body);
			content+="</div>";
			content+="</div>\r\n";

			long currentlevel=0;
			MessageThread thread;
			thread.Load(messageidstr,boardid);

			std::vector<MessageThread::threadnode> nodes=thread.GetNodes();
			if(nodes.size()>1)
			{
				content+="<ul class=\"messagethread\">";
				for(std::vector<MessageThread::threadnode>::const_iterator i=nodes.begin(); i!=nodes.end(); i++)
				{
					if((*i).m_level>currentlevel)
					{
						content+="<ul class=\"messagethread\">";
					}
					else if((*i).m_level<currentlevel)
					{
						content+="</ul>";
					}
					currentlevel=(*i).m_level;

					std::string tempstr="";
					StringFunctions::Convert((*i).m_messageid,tempstr);

					content+="<li>";
					if(tempstr!=messageidstr)
					{
						content+="<a href=\""+m_pagename+"?"+BuildQueryString(0,boardidstr,tempstr)+"\">"+SanitizeOutput((*i).m_subject)+"</a> - "+SanitizeOutput((*i).m_fromname);
					}
					else
					{
						content+=SanitizeOutput((*i).m_subject)+" - "+SanitizeOutput((*i).m_fromname);
					}
					content+="</li>\r\n";
				}
				while(currentlevel-->0)
				{
					content+="</ul>";
				}
				content+="</ul>\r\n";
			}
		}
	}

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}
