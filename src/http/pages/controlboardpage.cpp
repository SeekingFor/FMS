#include "../../../include/http/pages/controlboardpage.h"
#include "../../../include/stringfunctions.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ControlBoardPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
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
			StringFunctions::Convert((*queryvars.find("boardid")).second,boardid);

			st=m_db->Prepare("DELETE FROM tblAdministrationBoard WHERE BoardID=?;");
			st.Bind(0,boardid);
			st.Step();

			st=m_db->Prepare("DELETE FROM tblBoard WHERE BoardID=?;");
			st.Bind(0,boardid);
			st.Step();

			st=m_db->Prepare("DELETE FROM tblMessage WHERE MessageUUID IN (SELECT MessageUUID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE BoardID=?);");
			st.Bind(0,boardid);
			st.Step();

			st=m_db->Prepare("DELETE FROm tblMessageBoard WHERE BoardID=?;");
			st.Bind(0,boardid);
			st.Step();

		}
		if((*queryvars.find("formaction")).second=="addboard" && queryvars.find("boardname")!=queryvars.end() && (*queryvars.find("boardname")).second!="" && ValidateFormPassword(queryvars))
		{
			Poco::DateTime date;
			st=m_db->Prepare("INSERT INTO tblBoard(BoardName,DateAdded) VALUES(?,?);");
			st.Bind(0,(*queryvars.find("boardname")).second);
			st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
			if(st.Step(true))
			{
				boardid=st.GetLastInsertRowID();
				StringFunctions::Convert((*queryvars.find("changemessagetrust")).second,changemessagetrust);
				StringFunctions::Convert((*queryvars.find("changetrustlisttrust")).second,changetrustlisttrust);

				st=m_db->Prepare("INSERT INTO tblAdministrationBoard(BoardID,ModifyLocalMessageTrust,ModifyLocalTrustListTrust) VALUES(?,?,?);");
				st.Bind(0,boardid);
				st.Bind(1,changemessagetrust);
				st.Bind(2,changetrustlisttrust);
				st.Step();

			}
		}
	}

	content+="<h2>Control Boards</h2>";
	content+="<p class=\"paragraph\">";
	content+="These boards are special administration boards where sent messages will change the trust levels of the parent poster by ADDING these numbers to their current trust level.  These boards can not be used as regular boards, so make the name unique.  The change in trust levels can be negative or positive, but keep in mind that the minimum trust level is 0 and the maximum trust level is 100.  After the boards are created here, you may use your newreader to reply to a message to one or more of these boards, and the previous poster will have his trust levels changed as per the settings for that board.";
	content+="</p>";

	st=m_db->Prepare("SELECT tblBoard.BoardID,BoardName,ModifyLocalMessageTrust,ModifyLocalTrustListTrust FROM tblBoard INNER JOIN tblAdministrationBoard ON tblBoard.BoardID=tblAdministrationBoard.BoardID ORDER BY BoardName COLLATE NOCASE;");
	st.Step();

	content+="<table>";
	content+="<tr><th>Board Name</th><th>Change Message Trust</th><th>Change Trust List Trust</th></tr>\r\n";
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
		content+="<input type=\"submit\" value=\"Remove\">";
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
	content+="<input type=\"submit\" value=\"Add\">";
	content+="</form>";
	content+="</td>\r\n";
	content+="</tr>";
	content+="</table>";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
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
