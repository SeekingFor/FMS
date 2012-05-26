#include "../../../include/http/pages/controlboardpage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/board.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ControlBoardPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	std::string content="";
	int boardid;
	std::string boardidstr;
	std::string boardname;
	int changemessagetrust=0;
	int changetrustlisttrust=0;
	std::string changemessagetruststr;
	std::string changetrustlisttruststr;

	SQLite3DB::Statement st;

	if(queryvars.find("formaction")!=queryvars.end())
	{
		if((*queryvars.find("formaction")).second=="remove" && queryvars.find("boardid")!=queryvars.end() && ValidateFormPassword(queryvars))
		{
			int boardid=0;
			StringFunctions::Convert((*queryvars.find("boardid")).second.GetData(),boardid);

			st=m_db->Prepare("DELETE FROM tblAdministrationBoard WHERE BoardID=?;");
			st.Bind(0,boardid);
			st.Step();

			st=m_db->Prepare("DELETE FROM tblBoard WHERE BoardID=?;");
			st.Bind(0,boardid);
			st.Step();

			st=m_db->Prepare("DELETE FROM tblMessage WHERE MessageUUID IN (SELECT MessageUUID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE BoardID=? AND MessageUUID IS NOT NULL);");
			st.Bind(0,boardid);
			st.Step();

			st=m_db->Prepare("DELETE FROm tblMessageBoard WHERE BoardID=?;");
			st.Bind(0,boardid);
			st.Step();

		}
		if((*queryvars.find("formaction")).second=="addboard" && queryvars.find("boardname")!=queryvars.end() && (*queryvars.find("boardname")).second!="" && ValidateFormPassword(queryvars))
		{
			Poco::DateTime date;
			std::string boardname=(*queryvars.find("boardname")).second.GetData();
			boardname=Board::FixBoardName(boardname);
			st=m_db->Prepare("INSERT INTO tblBoard(BoardName,DateAdded) VALUES(?,?);");
			st.Bind(0,boardname);
			st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
			if(st.Step(true))
			{
				boardid=st.GetLastInsertRowID();
				StringFunctions::Convert((*queryvars.find("changemessagetrust")).second.GetData(),changemessagetrust);
				StringFunctions::Convert((*queryvars.find("changetrustlisttrust")).second.GetData(),changetrustlisttrust);

				st=m_db->Prepare("INSERT INTO tblAdministrationBoard(BoardID,ModifyLocalMessageTrust,ModifyLocalTrustListTrust) VALUES(?,?,?);");
				st.Bind(0,boardid);
				st.Bind(1,changemessagetrust);
				st.Bind(2,changetrustlisttrust);
				st.Step();

			}
		}
	}

	content+="<h2>"+m_trans->Get("web.page.controlboard.title")+"</h2>";
	content+="<p class=\"paragraph\">";
	content+=m_trans->Get("web.page.controlboard.instructions");
	content+="</p>";

	st=m_db->Prepare("SELECT tblBoard.BoardID,BoardName,ModifyLocalMessageTrust,ModifyLocalTrustListTrust FROM tblBoard INNER JOIN tblAdministrationBoard ON tblBoard.BoardID=tblAdministrationBoard.BoardID ORDER BY BoardName COLLATE NOCASE;");
	st.Step();

	content+="<table>";
	content+="<tr><th>"+m_trans->Get("web.page.controlboard.boardname")+"</th><th>"+m_trans->Get("web.page.controlboard.changemessagetrust")+"</th><th>"+m_trans->Get("web.page.controlboard.changetrustlisttrust")+"</th></tr>\r\n";
	while(st.RowReturned())
	{
		st.ResultText(0,boardidstr);
		st.ResultText(1,boardname);
		st.ResultText(2,changemessagetruststr);
		st.ResultText(3,changetrustlisttruststr);

		content+="<tr>";
		content+="<td>"+boardname+"</td>\r\n";
		content+="<td>"+changemessagetruststr+"</td>\r\n";
		content+="<td>"+changetrustlisttruststr+"</td>\r\n";
		content+="<td>";
		content+="<form name=\"frmremove\" method=\"POST\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"formaction\" value=\"remove\">";
		content+="<input type=\"hidden\" name=\"boardid\" value=\""+boardidstr+"\">";
		content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.controlboard.remove")+"\">";
		content+="</form>";
		content+="</td>";
		content+="</tr>\r\n";
		st.Step();
	}

	content+="<tr>";
	content+="<td>";
	content+="<form name=\"frmaddboard\" method=\"POST\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"addboard\">";
	content+="<input type=\"text\" name=\"boardname\">";
	content+="</td>\r\n<td>";
	content+="<input type=\"text\" name=\"changemessagetrust\" size=\"2\" maxlength=\"4\">";
	content+="</td>\r\n<td>";
	content+="<input type=\"text\" name=\"changetrustlisttrust\" size=\"2\" maxlength=\"4\">";
	content+="</td>\r\n<td>";
	content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.controlboard.add")+"\">";
	content+="</form>";
	content+="</td>\r\n";
	content+="</tr>";
	content+="</table>";

	return content;
}

const bool ControlBoardPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("controlboard.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
