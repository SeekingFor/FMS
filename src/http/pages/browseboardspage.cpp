#include "../../../include/http/pages/browseboardspage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string BrowseBoardsPage::BuildQueryString(const long startrow, const std::string &boardsearch, const std::string &sortby, const std::string &sortorder)
{
	std::string returnval="";
	std::string tempval="";

	if(startrow>=0)
	{
		StringFunctions::Convert(startrow,tempval);
		returnval+="startrow="+tempval;
	}

	if(boardsearch!="")
	{
		if(returnval!="")
		{
			returnval+="&";
		}
		returnval+="boardsearch="+boardsearch;
	}

	if(sortby!="")
	{
		if(returnval!="")
		{
			returnval+="&";
		}
		returnval+="sortby="+sortby;
		if(sortorder=="ASC" || sortorder=="DESC")
		{
			if(returnval!="")
			{
				returnval+="&";
			}
			returnval+="sortorder="+sortorder;
		}
		else
		{
			if(returnval!="")
			{
				returnval+="&";
			}
			returnval+="sortorder=ASC";
		}
	}

	return returnval;

}

const std::string BrowseBoardsPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";
	std::string sql="";
	std::string boardname="";
	int rowsperpage=25;
	std::string rowsperpagestr="26";	// 1 more than rowsperpage so we know if there are more boards
	long startrow=0;
	std::string startrowstr="0";
	int messagecount=0;
	std::string messagecountstr="0";
	std::string lastdate="";
	int count=0;
	std::string boardsearch="";
	std::string sortby="";
	std::string sortorder="";

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
	if(queryvars.find("boardsearch")!=queryvars.end())
	{
		boardsearch=(*queryvars.find("boardsearch")).second;
	}
	if(queryvars.find("sortby")!=queryvars.end() && queryvars.find("sortorder")!=queryvars.end())
	{
		sortby=(*queryvars.find("sortby")).second;
		sortorder=(*queryvars.find("sortorder")).second;
		if(sortby!="BoardName" && sortby!="MessageCount" && sortby!="LastMessage")
		{
			sortby="BoardName";
		}
		if(sortorder!="ASC" && sortorder!="DESC")
		{
			sortorder="ASC";
		}
	}
	else
	{
		sortby="BoardName";
		sortorder="ASC";
	}

	content="<h2>Browse Messages</h2>";
	content+="<form name=\"frmfilter\" method=\"post\" action=\""+m_pagename+"\">";
	content+="<input type=\"text\" name=\"boardsearch\" value=\""+SanitizeOutput(boardsearch)+"\">";
	content+="<input type=\"submit\" value=\"Filter\">";
	content+="</form>";

	content+="<table class=\"small90\">";
	content+="<tr>";
	content+="<th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,boardsearch,"BoardName",ReverseSort("BoardName",sortby,sortorder))+"\">Board</a></th>";
	content+="<th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,boardsearch,"MessageCount",ReverseSort("MessageCount",sortby,sortorder))+"\">Message Count</a></th>";
	content+="<th><a href=\""+m_pagename+"?"+BuildQueryString(startrow,boardsearch,"LastMessage",ReverseSort("LastMessage",sortby,sortorder))+"\">Last Message</a></th>";
	content+="</tr>";

	sql="SELECT tblBoard.BoardID, tblBoard.BoardName, COUNT(tblMessageBoard.MessageID) AS 'MessageCount', MAX(tblMessage.MessageDate || ' ' || tblMessage.MessageTime) AS 'LastMessage'";
	sql+="FROM tblBoard LEFT JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID ";
	sql+="LEFT JOIN tblMessage ON tblMessageBoard.MessageID=tblMessage.MessageID ";
	sql+="WHERE (tblMessageBoard.MessageID>=0 OR tblMessageBoard.MessageID IS NULL) ";
	if(boardsearch!="")
	{
		sql+="AND tblBoard.BoardName LIKE '%' || ? || '%' ";
	}
	sql+="GROUP BY tblBoard.BoardID ";
	sql+="ORDER BY "+sortby+" COLLATE NOCASE "+sortorder+" ";
	sql+="LIMIT "+startrowstr+","+rowsperpagestr+";";
	
	SQLite3DB::Statement st=m_db->Prepare(sql);
	if(boardsearch!="")
	{
		st.Bind(0,boardsearch);
	}
	
	st.Step();
	while(st.RowReturned() && count++<rowsperpage)
	{
		boardname="";
		messagecount=0;
		lastdate="";
		std::string boardidstr="0";
		st.ResultText(0,boardidstr);
		st.ResultText(1,boardname);
		st.ResultInt(2,messagecount);
		st.ResultText(3,lastdate);

		StringFunctions::Convert(messagecount,messagecountstr);

		content+="<tr>";
		content+="<td><a href=\"browsemessages.htm?boardid="+boardidstr+"\">"+SanitizeOutput(boardname)+"</a></td>";
		content+="<td>"+SanitizeOutput(messagecountstr)+"</td>";
		content+="<td>"+SanitizeOutput(lastdate)+"</td>";
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
			content+="<td><a href=\"boardsbrowse.htm?"+BuildQueryString(thisstartrow,boardsearch,sortby,sortorder)+"\">&lt;--</a></td>";
		}
		else
		{
			content+="<td></td>";
		}

		content+="<td></td>";

		if(st.RowReturned())
		{
			int thisstartrow=startrow+rowsperpage;
			content+="<td><a href=\"boardsbrowse.htm?"+BuildQueryString(thisstartrow,boardsearch,sortby,sortorder)+"\">--&gt;</a></td>";
		}

		content+="</tr>";
	}
	content+="</table>";
	
	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const std::string BrowseBoardsPage::ReverseSort(const std::string &sortname, const std::string &currentsortby, const std::string &currentsortorder)
{
	if(sortname==currentsortby)
	{
		if(currentsortorder=="ASC")
		{
			return "DESC";
		}
		else
		{
			return "ASC";
		}
	}
	else
	{
		return currentsortorder;
	}
}
