#include "../../../include/http/pages/boardspage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/board.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string BoardsPage::BuildQueryString(const long startrow, const std::string &boardsearch)
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

	return returnval;

}

const std::string BoardsPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	int boardcount=0;
	std::string content="";
	int rownum=0;
	int rowsperpage=25;
	std::string rowsperpagestr="25";
	int startrow=0;
	std::string startrowstr="0";
	std::string boardsearch="";
	std::string sql="";
	Poco::DateTime now;

	if(queryvars.find("formaction")!=queryvars.end())
	{
		if((*queryvars.find("formaction")).second=="addboard" && queryvars.find("boardname")!=queryvars.end() && queryvars.find("boarddescription")!=queryvars.end() && ValidateFormPassword(queryvars))
		{
			std::string boardname="";
			std::string boarddescription="";

			boardname=(*queryvars.find("boardname")).second.GetData();
			boardname=Board::FixBoardName(boardname);
			boarddescription=(*queryvars.find("boarddescription")).second.GetData();

			if(boardname!="")
			{
				SQLite3DB::Statement addst=m_db->Prepare("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded,AddedMethod) VALUES(?,?,?,?);");
				addst.Bind(0,boardname);
				addst.Bind(1,boarddescription);
				addst.Bind(2,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
				addst.Bind(3,"Added manually");
				addst.Step();
			}
		}
		if((*queryvars.find("formaction")).second=="remove0messages" && ValidateFormPassword(queryvars))
		{
			m_db->Execute("DELETE FROM tblBoard WHERE BoardID IN (SELECT BoardID FROM vwBoardStats WHERE MessageCount=0 AND BoardID NOT IN (SELECT BoardID FROM tblAdministrationBoard));");
		}
		if((*queryvars.find("formaction")).second=="update" && ValidateFormPassword(queryvars))
		{
			int boardid;
			std::vector<std::string> boardids;
			std::vector<std::string> olddescriptions;
			std::vector<std::string> descriptions;
			std::vector<std::string> oldsavemessages;
			std::vector<std::string> savemessages;
			std::vector<std::string> oldforums;
			std::vector<std::string> forums;

			CreateArgArray(queryvars,"boardid",boardids);
			CreateArgArray(queryvars,"oldboarddescription",olddescriptions);
			CreateArgArray(queryvars,"boarddescription",descriptions);
			CreateArgArray(queryvars,"oldsavereceivedmessages",oldsavemessages);
			CreateArgArray(queryvars,"savereceivedmessages",savemessages);
			CreateArgArray(queryvars,"oldforum",oldforums);
			CreateArgArray(queryvars,"forum",forums);

			olddescriptions.resize(boardids.size(),"");
			descriptions.resize(boardids.size(),"");
			oldsavemessages.resize(boardids.size(),"");
			savemessages.resize(boardids.size(),"");
			oldforums.resize(boardids.size(),"");
			forums.resize(boardids.size(),"");

			SQLite3DB::Statement updatest=m_db->Prepare("UPDATE tblBoard SET BoardDescription=?, SaveReceivedMessages=?, Forum=? WHERE BoardID=?;");
			
			for(int i=0; i<boardids.size(); i++)
			{
				if(olddescriptions[i]!=descriptions[i] || oldsavemessages[i]!=savemessages[i] || oldforums[i]!=forums[i])
				{
					updatest.Bind(0,descriptions[i]);
					if(savemessages[i]!="true")
					{
						updatest.Bind(1,"false");
					}
					else
					{
						updatest.Bind(1,"true");
					}
					if(forums[i]!="true")
					{
						updatest.Bind(2,"false");
					}
					else
					{
						updatest.Bind(2,"true");
					}
					boardid=0;
					StringFunctions::Convert(boardids[i],boardid);
					updatest.Bind(3,boardid);
					updatest.Step();
					updatest.Reset();
				}
			}

		}
	}

	// if startrow is specified
	if(queryvars.find("startrow")!=queryvars.end())
	{
		startrowstr=(*queryvars.find("startrow")).second.GetData();
		// convert back and forth, just in case a number wasn't passed in startrow
		StringFunctions::Convert(startrowstr,startrow);
		if(startrow<0)
		{
			startrow=0;
		}
		StringFunctions::Convert(startrow,startrowstr);
	}

	// if we are searching by name
	if(queryvars.find("boardsearch")!=queryvars.end())
	{
		boardsearch=(*queryvars.find("boardsearch")).second.GetData();
	}

	content+="<h2>"+m_trans->Get("web.page.boards.title")+"</h2>";

	sql="SELECT COUNT(*) FROM tblBoard WHERE BoardID NOT IN (SELECT BoardID FROM tblAdministrationBoard)";
	if(boardsearch!="")
	{
		sql+=" AND (BoardName LIKE '%' || ? || '%' OR BoardDescription LIKE '%' || ? || '%')";
	}
	sql+=";";
	SQLite3DB::Statement st=m_db->Prepare(sql);
	if(boardsearch!="")
	{
		st.Bind(0,boardsearch);
		st.Bind(1,boardsearch);
	}
	st.Step();
	if(st.RowReturned())
	{
		st.ResultInt(0,boardcount);
	}
	st.Finalize();


	sql="SELECT BoardID,BoardName,BoardDescription,SaveReceivedMessages,AddedMethod,Forum,MessageCount FROM tblBoard WHERE BoardID NOT IN (SELECT BoardID FROM tblAdministrationBoard)";
	if(boardsearch!="")
	{
		sql+=" AND (BoardName LIKE '%' || ? || '%' OR BoardDescription LIKE '%' || ? || '%')";
	}
	sql+=" ORDER BY BoardName COLLATE NOCASE";
	sql+=" LIMIT "+startrowstr+","+rowsperpagestr+";";

	st=m_db->Prepare(sql);
	if(boardsearch!="")
	{
		st.Bind(0,boardsearch);
		st.Bind(1,boardsearch);
	}
	st.Step();

	content+="<table class=\"small90\">";

	content+="<tr>";
	content+="<td colspan=\"3\"><center>";
	content+="<form name=\"frmboardsearch\" action=\"boards.htm\" method=\"POST\"><input type=\"text\" name=\"boardsearch\" value=\""+SanitizeOutput(boardsearch)+"\">"+CreateFormPassword()+"<input type=\"submit\" value=\""+m_trans->Get("web.page.boards.search")+"\"></form>";
	content+="</center></td>";
	content+="</tr>";

	content+="<tr>";
	content+="<td colspan=\"3\"><center>";
	content+="<form name=\"frmremoveboard\" action=\"boards.htm\" method=\"POST\">"+CreateFormPassword()+"<input type=\"hidden\" name=\"formaction\" value=\"remove0messages\">"+m_trans->Get("web.page.boards.remove0messages")+"<input type=\"submit\" value=\""+m_trans->Get("web.page.boards.remove")+"\"></form>";
	content+="</center></td>";
	content+="</tr>";

	content+="<tr>";
	content+="<td><form name=\"frmaddboard\" method=\"POST\">"+CreateFormPassword()+"<input type=\"hidden\" name=\"formaction\" value=\"addboard\"><input type=\"text\" name=\"boardname\" maxlength=\""MAX_BOARD_NAME_LENGTH_STR"\"></td><td><input type=\"text\" name=\"boarddescription\" size=\"40\" maxlength=\""MAX_BOARD_DESCRIPTION_LENGTH_STR"\"></td><td><input type=\"submit\" value=\""+m_trans->Get("web.page.boards.addboard")+"\"></form></td>";
	content+="</tr>";

	content+="<tr><td colspan=\"4\"><hr><form name=\"frmboards\" method=\"POST\"><input type=\"hidden\" name=\"formaction\" value=\"update\">"+CreateFormPassword()+"</td></tr>";
	content+="<tr>";
	content+="<th>"+m_trans->Get("web.page.boards.name")+"</th><th>"+m_trans->Get("web.page.boards.description")+"</th><th>"+m_trans->Get("web.page.boards.messagecount")+"</th><th>"+m_trans->Get("web.page.boards.savereceivedmessages")+"</th><th>"+m_trans->Get("web.page.boards.forum")+"</th><th>"+m_trans->Get("web.page.boards.addedmethod")+"</th>";
	content+="</tr>";	
	while(st.RowReturned() && rownum<rowsperpage)
	{
		std::string rownumstr="";
		std::string boardidstr="";
		std::string boardname="";
		std::string boarddescription="";
		std::string savereceivedmessages="";
		std::string addedmethod="";
		std::string forum="";
		std::string messagecountstr="";

		st.ResultText(0,boardidstr);
		st.ResultText(1,boardname);
		st.ResultText(2,boarddescription);
		st.ResultText(3,savereceivedmessages);
		st.ResultText(4,addedmethod);
		st.ResultText(5,forum);
		st.ResultText(6,messagecountstr);

		StringFunctions::Convert(rownum,rownumstr);

		content+="<tr>";
		content+="<td>"+SanitizeOutput(boardname)+"</td>";
		content+="<td><input type=\"hidden\" name=\"boardid["+rownumstr+"]\" value=\""+boardidstr+"\">";
		content+="<input type=\"hidden\" name=\"oldboarddescription["+rownumstr+"]\" value=\""+StringFunctions::Replace(SanitizeOutput(boarddescription),"&nbsp;"," ")+"\">";
		content+="<input type=\"text\" name=\"boarddescription["+rownumstr+"]\" value=\""+SanitizeOutput(boarddescription)+"\" size=\"40\" maxlength=\""MAX_BOARD_DESCRIPTION_LENGTH_STR"\"></td>";
		content+="<td style=\"text-align:right;\">"+messagecountstr+"</td>";
		content+="<td>";
		content+="<input type=\"hidden\" name=\"oldsavereceivedmessages["+rownumstr+"]\" value=\""+savereceivedmessages+"\">";
		content+="<input type=\"checkbox\" name=\"savereceivedmessages["+rownumstr+"]\" value=\"true\"";
		if(savereceivedmessages=="true")
		{
			content+=" CHECKED";
		}
		content+=">";
		content+="</td>";
		content+="<td>";
		content+="<input type=\"hidden\" name=\"oldforum["+rownumstr+"]\" value=\""+forum+"\">";
		content+="<input type=\"checkbox\" name=\"forum["+rownumstr+"]\" value=\"true\"";
		if(forum=="true")
		{
			content+=" CHECKED";
		}
		content+=">";
		content+="</td>";
		content+="<td>"+SanitizeOutput(addedmethod)+"</td>";
		content+="</tr>\r\n";
		st.Step();
		rownum++;
	}

	if(startrow>0 || startrow+rowsperpage<boardcount)
	{
		std::string tempstr;
		int cols=0;

		content+="<tr>";
		if(startrow>0)
		{
			StringFunctions::Convert(startrow-rowsperpage,tempstr);
			content+="<td colspan=\"2\" style=\"text-align:left;\"><a href=\"boards.htm?"+BuildQueryString(startrow-rowsperpage,boardsearch)+"\">"+m_trans->Get("web.page.boards.previouspage")+"</a></td>";
			cols+=2;
		}
		if(startrow+rowsperpage<boardcount)
		{
			while(cols<5)
			{
				content+="<td></td>";
				cols++;
			}
			content+="<td colspan=\"1\" style=\"text-align:left;\"><a href=\"boards.htm?"+BuildQueryString(startrow+rowsperpage,boardsearch)+"\">"+m_trans->Get("web.page.boards.nextpage")+"</a></td>";
		}
		content+="</tr>";
	}

	content+="<tr>";
	content+="<td colspan=\"4\"><center><input type=\"submit\" value=\""+m_trans->Get("web.page.boards.update")+"\"></center></form></td>";
	content+="</tr>";
	content+="</table>";
	content+="<p class=\"paragraph\">";
	content+=m_trans->Get("web.page.boards.saveinstructions");
	content+="</p>";

	return content;
}

const bool BoardsPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("boards.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
