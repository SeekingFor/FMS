#include "../../../include/http/pages/showpendingmessagepage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/global.h"
#include "../../../include/fmsapp.h"
#include "../../../include/option.h"
#include "../../../include/localidentity.h"
#include "../../../include/freenet/messagexml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ShowPendingMessagePage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="delete" && ValidateFormPassword(queryvars))
	{
		m_log->information("User requested to delete message "+(*queryvars.find("uuid")).second);
		m_db->Execute("DELETE FROM tblMessageInserts WHERE MessageUUID=\""+(*queryvars.find("uuid")).second+"\"");
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, MessageXML, SendDate, MessageUUID FROM tblMessageInserts WHERE Inserted='false';");
	st.Step();
	int msgcount=0;
	std::string tblcontent="";
	std::string content="";
	tblcontent+="<table><tr><td>Identity</td><td>Boards</td><td>Subject</td><td>Time</td></tr>";
	while (st.RowReturned())
	{	
		int identityid=0;
		std::string time("");
		std::string uuid("");
		std::string subject("");

		st.ResultInt(0,identityid);
		st.ResultText(2,time);
		st.ResultText(3, uuid);

		LocalIdentity ident(m_db); //found a canned way, thanks SomeDude!
		ident.Load(identityid);

		tblcontent+="<tr><td>";
		tblcontent+=SanitizeOutput(ident.GetName())+"</td><td>";
		//yes, the next bit sucks but there's no better way to do it (that I could find)
		//we will look at the message XML to find the board(s) posted to.... 
		std::string xml="";
		st.ResultText(1,xml);
		MessageXML mxml;
		mxml.ParseXML(xml);
		std::vector<std::string> boards=mxml.GetBoards();
		std::vector<std::string>::iterator iter;
		for (iter=boards.begin(); iter!=boards.end(); iter++) tblcontent+=*iter+", ";
		tblcontent.erase(tblcontent.length()-2); //strip final ", "
		tblcontent+="</td><td>";
		subject=mxml.GetSubject();
		tblcontent+=subject;
		tblcontent+="</td><td>";
		tblcontent+=time+"</td><td>";
		//button
		tblcontent+="<form name=\"frmdelete\" method=\"POST\">";
		tblcontent+=CreateFormPassword();
		tblcontent+="<input type=\"hidden\" name=\"formaction\" value=\"delete\">";
		tblcontent+="<input type=\"hidden\" name=\"uuid\" value=\""+uuid+"\">";
		tblcontent+="<input type=\"submit\" value=\"Delete Message\">";
		tblcontent+="</form>";
		tblcontent+="</td></tr>";
		st.Step();
		msgcount++;
	}
	tblcontent+="</table>";

	std::string msgcountstr("");
	StringFunctions::Convert(msgcount,msgcountstr);
	content="<h2>"+msgcountstr+" messages waiting to be inserted</h2>";

	content+=tblcontent;

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}
