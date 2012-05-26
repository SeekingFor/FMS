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

const std::string ShowPendingMessagePage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="delete" && ValidateFormPassword(queryvars))
	{
		m_log->information("User requested to delete message "+(*queryvars.find("uuid")).second.GetData());
		SQLite3DB::Statement st=m_db->Prepare("DELETE FROM tblMessageInserts WHERE MessageUUID=?");
		st.Bind(0, (*queryvars.find("uuid")).second.GetData());
		st.Step();
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, MessageXML, SendDate, MessageUUID FROM tblMessageInserts WHERE Inserted='false';");
	st.Step();
	int msgcount=0;
	std::string tblcontent="";
	std::string content="";
	tblcontent+="<table><tr><td>"+m_trans->Get("web.page.pendingmessages.identity")+"</td><td>"+m_trans->Get("web.page.pendingmessages.boards")+"</td><td>"+m_trans->Get("web.page.pendingmessages.subject")+"</td><td>"+m_trans->Get("web.page.pendingmessages.time")+"</td></tr>";
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
		tblcontent+=SanitizeOutput(subject);
		tblcontent+="</td><td>";
		tblcontent+=time+"</td><td>";
		//button
		tblcontent+="<form name=\"frmdelete\" method=\"POST\">";
		tblcontent+=CreateFormPassword();
		tblcontent+="<input type=\"hidden\" name=\"formaction\" value=\"delete\">";
		tblcontent+="<input type=\"hidden\" name=\"uuid\" value=\""+uuid+"\">";
		tblcontent+="<input type=\"submit\" value=\""+m_trans->Get("web.page.pendingmessages.deletemessage")+"\">";
		tblcontent+="</form>";
		tblcontent+="</td></tr>";
		st.Step();
		msgcount++;
	}
	tblcontent+="</table>";

	std::string msgcountstr("");
	StringFunctions::Convert(msgcount,msgcountstr);
	content="<h2>"+msgcountstr+" "+m_trans->Get("web.page.pendingmessages.messageswaiting")+"</h2>";

	content+=tblcontent;

	return content;
}
