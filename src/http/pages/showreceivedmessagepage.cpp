#include "../../../include/http/pages/showreceivedmessagepage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/global.h"
#include "../../../include/fmsapp.h"
#include "../../../include/option.h"
#include "../../../include/localidentity.h"
#include "../../../include/freenet/messagexml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

static const std::string BuildQueryString(const long startrow, const std::string &identityid)
{
	std::string returnval="";
	std::string tempval="";

	if(startrow>=0)
	{
		StringFunctions::Convert(startrow,tempval);
		returnval+="startrow="+tempval;
	}

	if(identityid!="")
	{
		if(returnval!="")
		{
			returnval+="&";
		}
		returnval+="identityid="+identityid;
	}

	return returnval;

}

const std::string ShowReceivedMessagePage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	int rowscount=0;
	std::string rowscountstr="0";
	std::string identityidstr="";
	int startrow=0;
	std::string startrowstr="0";
	std::string rowsperpagestr="25";
	int rowsperpage=25;
	std::string tblcontent="";
	std::string content="";
	std::string identityname="";

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

	if(queryvars.find("identityid")!=queryvars.end() && (*queryvars.find("identityid")).second!="")
	{
		identityidstr=(*queryvars.find("identityid")).second.GetData();
	}

	std::string sql = "SELECT COUNT(*) FROM tblMessage";
	if(identityidstr!="")
	{
		sql+=" WHERE IdentityID=?";
	}
	SQLite3DB::Statement st=m_db->Prepare(sql+";");
	if(identityidstr!="")
	{
		st.Bind(0,identityidstr);
	}
	st.Step();
	if(st.RowReturned())
	{
		st.ResultInt(0,rowscount);
		st.ResultText(0,rowscountstr);
	}
	st.Finalize();

	SQLite3DB::Statement board=m_db->Prepare("SELECT BoardName FROM tblBoard WHERE BoardID=? LIMIT 1;");
	SQLite3DB::Statement topic=m_db->Prepare("SELECT ThreadID FROM tblThreadPost WHERE MessageID=? LIMIT 1;");

	sql="SELECT i.Name, m.MessageID, m.MessageDate, m.MessageTime, m.Subject, m.IdentityID, m.ReplyBoardID";
	sql+=" FROM tblMessage m, tblIdentity i";
	sql+=" WHERE m.IdentityID=i.IdentityID";
	if(identityidstr!="")
	{
		sql+=" AND m.IdentityID=?";
	}
	sql+=" ORDER by MessageID DESC LIMIT "+startrowstr+", "+rowsperpagestr+";";
	st=m_db->Prepare(sql);
	if(identityidstr!="")
	{
		st.Bind(0,identityidstr);
	}
	st.Step();
	tblcontent+="<table width=\"100%\"><tr><td>"+m_trans->Get("web.page.receivedmessages.identity")+"</td>";
	tblcontent+="<td>"+m_trans->Get("web.page.receivedmessages.board")+"</td>";
	tblcontent+="<td>"+m_trans->Get("web.page.receivedmessages.subject")+"</td>";
	tblcontent+="<td>"+m_trans->Get("web.page.receivedmessages.time")+"</td></tr>";
	while (st.RowReturned())
	{
		std::string msidstr("");
		std::string day("");
		std::string tim("");
		std::string subject("");
		std::string ident="";
		std::string idstr="";
		std::string bid="", bname="???";

		st.ResultText(0,ident);
		st.ResultText(0,identityname);
		st.ResultText(1,msidstr);
		st.ResultText(2,day);
		st.ResultText(3,tim);
		st.ResultText(4,subject);
		st.ResultText(5,idstr);
		st.ResultText(6,bid);

		tblcontent+="<tr class=\"smaller\"><td>";
		tblcontent+="<a href=\"";
		if (identityidstr!="")  tblcontent+="peerdetails.htm";
		tblcontent+="?identityid="+idstr+"\">"+SanitizeOutput(ident)+"</a></td>";

		board.Bind(0,bid);
		board.Step();
		if(board.RowReturned())  board.ResultText(0,bname);
		board.Reset();
		tblcontent+="<td><a href=\"forumthreads.htm?boardid="+bid+"\">"+SanitizeOutput(bname)+"</a></td><td>";

		subject=SanitizeOutput(subject);

		topic.Bind(0, msidstr);
		topic.Step();
		if(topic.RowReturned())
		{
			std::string tid("");
			topic.ResultText(0,tid);
			subject="<a href=\"forumviewthread.htm?threadid="+tid+"&boardid="+bid+"#"+msidstr+"\">"+subject+"</a>";
		}
		topic.Reset();

		tblcontent+=subject;
		tblcontent+="</td><td>"+day+" "+tim+"</td>";
		tblcontent+="</tr>";

		st.Step();
	}

	if(startrow>0 || startrow+rowsperpage<rowscount)
	{
		std::string tempstr;
		int cols=0;

		tblcontent+="<tr>";
		if(startrow>0)
		{
			StringFunctions::Convert(startrow-rowsperpage,tempstr);
			tblcontent+="<td colspan=\"2\" style=\"text-align:left;\"><a href=\"?"+BuildQueryString(startrow-rowsperpage,identityidstr)+"\">"+m_trans->Get("web.page.receivedmessages.prevpage")+"</a></td>";
			cols+=2;
		}
		if(startrow+rowsperpage<rowscount)
		{
			while(cols<3)
			{
				tblcontent+="<td></td>";
				cols++;
			}
			tblcontent+="<td colspan=\"1\" style=\"text-align:left;\"><a href=\"?"+BuildQueryString(startrow+rowsperpage,identityidstr)+"\">"+m_trans->Get("web.page.receivedmessages.nextpage")+"</a></td>";
		}
		tblcontent+="</tr>";
	}

	tblcontent+="</table>";
	content="<h2>"+rowscountstr+" "+m_trans->Get("web.page.receivedmessages.msgsreceived");
	if(identityidstr!="")
	{
		content+= " "+m_trans->Get("web.page.receivedmessages.from")+" "+identityname+" (<a href=\"?\">"+m_trans->Get("web.page.receivedmessages.showallusers")+"</a>)";
	}
	content+="</h2>";

	content+=tblcontent;

	return content;
}
