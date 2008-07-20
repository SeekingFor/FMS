#include "../../../include/http/pages/boardspage.h"
#include "../../../include/stringfunctions.h"

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

const std::string BoardsPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
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

			boardname=(*queryvars.find("boardname")).second;
			StringFunctions::LowerCase(boardname,boardname);
			boarddescription=(*queryvars.find("boarddescription")).second;

			SQLite3DB::Statement addst=m_db->Prepare("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded,AddedMethod) VALUES(?,?,?,?);");
			addst.Bind(0,boardname);
			addst.Bind(1,boarddescription);
			addst.Bind(2,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
			addst.Bind(3,"Added manually");
			addst.Step();
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

			CreateArgArray(queryvars,"boardid",boardids);
			CreateArgArray(queryvars,"oldboarddescription",olddescriptions);
			CreateArgArray(queryvars,"boarddescription",descriptions);
			CreateArgArray(queryvars,"oldsavereceivedmessages",oldsavemessages);
			CreateArgArray(queryvars,"savereceivedmessages",savemessages);

			olddescriptions.resize(boardids.size(),"");
			descriptions.resize(boardids.size(),"");
			oldsavemessages.resize(boardids.size(),"");
			savemessages.resize(boardids.size(),"");

			SQLite3DB::Statement updatest=m_db->Prepare("UPDATE tblBoard SET BoardDescription=?, SaveReceivedMessages=? WHERE BoardID=?;");
			
			for(int i=0; i<boardids.size(); i++)
			{
				if(olddescriptions[i]!=descriptions[i] || oldsavemessages[i]!=savemessages[i])
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
					boardid=0;
					StringFunctions::Convert(boardids[i],boardid);
					updatest.Bind(2,boardid);
					updatest.Step();
					updatest.Reset();
				}
			}

		}
	}

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

	// if we are searching by name
	if(queryvars.find("boardsearch")!=queryvars.end())
	{
		boardsearch=(*queryvars.find("boardsearch")).second;
	}

	content+="<h2>Boards</h2>";

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


	sql="SELECT BoardID,BoardName,BoardDescription,SaveReceivedMessages,AddedMethod FROM tblBoard WHERE BoardID NOT IN (SELECT BoardID FROM tblAdministrationBoard)";
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

	content+="<table>";

	content+="<tr>";
	content+="<td colspan=\"3\"><center>";
	content+="<form name=\"frmboardsearch\" action=\"boards.htm\" method=\"POST\"><input type=\"text\" name=\"boardsearch\" value=\""+SanitizeOutput(boardsearch)+"\">"+CreateFormPassword()+"<input type=\"submit\" value=\"Search\"></form>";
	content+="</center></td>";
	content+="</tr>";

	content+="<tr>";
	content+="<td colspan=\"3\"><center>";
	content+="<form name=\"frmremoveboard\" action=\"boards.htm\" method=\"POST\">"+CreateFormPassword()+"<input type=\"hidden\" name=\"formaction\" value=\"remove0messages\">Remove boards with 0 messages<input type=\"submit\" value=\"Remove\"></form>";
	content+="</center></td>";
	content+="</tr>";

	content+="<tr>";
	content+="<td><form name=\"frmaddboard\" method=\"POST\">"+CreateFormPassword()+"<input type=\"hidden\" name=\"formaction\" value=\"addboard\"><input type=\"text\" name=\"boardname\"></td><td><input type=\"text\" name=\"boarddescription\" size=\"40\" maxlength=\"50\"></td><td><input type=\"submit\" value=\"Add Board\"></form></td>";
	content+="</tr>";

	content+="<tr><td colspan=\"4\"><hr><form name=\"frmboards\" method=\"POST\"><input type=\"hidden\" name=\"formaction\" value=\"update\">"+CreateFormPassword()+"</td></tr>";
	content+="<tr>";
	content+="<th>Name</th><th>Description</th><th>Save Received Messages *</th><th>Added Method</th>";
	content+="</tr>";	
	while(st.RowReturned() && rownum<rowsperpage)
	{
		std::string rownumstr="";
		std::string boardidstr="";
		std::string boardname="";
		std::string boarddescription="";
		std::string savereceivedmessages="";
		std::string addedmethod="";

		st.ResultText(0,boardidstr);
		st.ResultText(1,boardname);
		st.ResultText(2,boarddescription);
		st.ResultText(3,savereceivedmessages);
		st.ResultText(4,addedmethod);

		StringFunctions::Convert(rownum,rownumstr);

		content+="<tr>";
		content+="<td>"+SanitizeOutput(boardname)+"</td>";
		content+="<td><input type=\"hidden\" name=\"boardid["+rownumstr+"]\" value=\""+boardidstr+"\">";
		content+="<input type=\"hidden\" name=\"oldboarddescription["+rownumstr+"]\" value=\""+StringFunctions::Replace(SanitizeOutput(boarddescription),"&nbsp;"," ")+"\">";
		content+="<input type=\"text\" name=\"boarddescription["+rownumstr+"]\" value=\""+SanitizeOutput(boarddescription)+"\" size=\"40\" maxlength=\"50\"></td>";
		content+="<td>";
		content+="<input type=\"hidden\" name=\"oldsavereceivedmessages["+rownumstr+"]\" value=\""+savereceivedmessages+"\">";
		content+="<input type=\"checkbox\" name=\"savereceivedmessages["+rownumstr+"]\" value=\"true\"";
		if(savereceivedmessages=="true")
		{
			content+=" CHECKED";
		}
		content+=">";
		content+="</td>";
		content+="<td class=\"smaller\">"+SanitizeOutput(addedmethod)+"</td>";
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
			content+="<td colspan=\"1\" align=\"left\"><a href=\"boards.htm?"+BuildQueryString(startrow-rowsperpage,boardsearch)+"\"><-- Previous Page</a></td>";
			cols+=1;
		}
		if(startrow+rowsperpage<boardcount)
		{
			while(cols<3)
			{
				content+="<td></td>";
				cols++;
			}
			content+="<td colspan=\"1\" align=\"right\"><a href=\"boards.htm?"+BuildQueryString(startrow+rowsperpage,boardsearch)+"\">Next Page --></a></td>";
		}
		content+="</tr>";
	}

	content+="<tr>";
	content+="<td colspan=\"4\"><center><input type=\"submit\" value=\"Update\"></center></form></td>";
	content+="</tr>";
	content+="</table>";
	content+="<p class=\"paragraph\">";
	content+="* If you uncheck this box, any new messages you download that are posted to this board will be discarded.  When multiple local identities are used, it is best not to discard messages from any boards, as identifying which identities are the same person is much easier when their message lists are missing messages from the same boards.";
	content+="</p>";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
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
