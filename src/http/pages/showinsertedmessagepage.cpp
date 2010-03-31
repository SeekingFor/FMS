#include "../../../include/http/pages/showinsertedmessagepage.h"
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

const std::string ShowInsertedMessagePage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	int rowscount=0;
	std::string rowscountstr="0";
	Option option(m_db);
	std::string fproxyprotocol="http";
	std::string fproxyhost="127.0.0.1";
	std::string fproxyport="8888";
	std::string identityidstr="";
	int startrow=0;
	std::string startrowstr="0";
	std::string rowsperpagestr="25";
	int rowsperpage=25;
	std::string tblcontent="";
	std::string content="";
	std::string messagebase("fms");

	option.Get("FProxyProtocol",fproxyprotocol);
	option.Get("FProxyHost",fproxyhost);
	option.Get("FProxyPort",fproxyport);
	option.Get("MessageBase",messagebase);

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

	std::string sql = "SELECT COUNT(*) FROM tblMessageInserts WHERE Inserted='true'";
	if(identityidstr!="")
	{
		sql+=" AND LocalIdentityID=?";
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

	sql="SELECT LocalIdentityID, Day, InsertIndex, MessageUUID, SendDate, MessageXML FROM tblMessageInserts WHERE Inserted='true'";
	if(identityidstr!="")
	{
		sql+=" AND LocalIdentityID=?";
	}
	sql+=" ORDER by SendDate DESC LIMIT "+startrowstr+", "+rowsperpagestr+";";
	st=m_db->Prepare(sql);
	st.Bind(0,identityidstr);
	SQLite3DB::Statement stmsg=m_db->Prepare("SELECT m.MessageID, tp.ThreadID, m.ReplyBoardID FROM tblMessage m, tblThreadPost tp WHERE m.MessageID=tp.MessageID AND m.MessageUUID=?;");
	SQLite3DB::Statement stgot=m_db->Prepare("SELECT MessageID FROM tblMessage WHERE MessageUUID=? LIMIT 1;");
	st.Step();
	tblcontent+="<table width=\"100%\"><tr><td>"+m_trans->Get("web.page.insertedmessages.identity")+"</td>";
	tblcontent+="<td>"+m_trans->Get("web.page.insertedmessages.boards")+"</td>";
	tblcontent+="<td>"+m_trans->Get("web.page.insertedmessages.subject")+"</td>";
	tblcontent+="<td>"+m_trans->Get("web.page.insertedmessages.senton")+"</td></tr>";
	while (st.RowReturned())
	{
		int identityid=0;
		std::string idstr("");
		std::string day("");
		std::string idx("");
		std::string time("");
		std::string uuid("");
		std::string subject("");

		st.ResultInt(0,identityid);
		st.ResultText(0,idstr);
		st.ResultText(1,day);
		st.ResultText(2,idx);
		st.ResultText(3,uuid);
		st.ResultText(4,time);

		LocalIdentity ident(m_db); //found a canned way, thanks SomeDude!
		ident.Load(identityid);

		tblcontent+="<tr class=\"smaller\"><td>";
		tblcontent+="<a href=\"";
		tblcontent+="?identityid="+idstr+"\">"+SanitizeOutput(ident.GetName())+"</a></td><td>";
		//yes, the next bit sucks but there's no better way to do it (that I could find)
		//we will look at the message XML to find the board(s) posted to....
		std::string xml="";
		st.ResultText(5,xml);
		MessageXML mxml;
		mxml.ParseXML(xml);
		std::vector<std::string> boards=mxml.GetBoards();
		std::vector<std::string>::iterator iter;
		tblcontent+= "  ";
		for(iter=boards.begin(); iter!=boards.end(); iter++)
		{
			tblcontent+=*iter+", ";
		}
		tblcontent.erase(tblcontent.length()-2); //strip final ", "
		tblcontent+="</td><td>";
		subject=SanitizeOutput(mxml.GetSubject());

		stmsg.Bind(0,uuid);
		stmsg.Step();
		if(stmsg.RowReturned())
		{
			std::string mid("");
			std::string tid("");
			std::string bid("");
			stmsg.ResultText(0,mid);
			stmsg.ResultText(1,tid);
			stmsg.ResultText(2,bid);
			subject="<a href=\"forumviewthread.htm?threadid="+tid+"&boardid="+bid+"#"+mid+"\">"+subject+"</a>";
		}
		else
		{
			subject="<a target=\"_blank\" href=\""+fproxyprotocol+"://"+fproxyhost+":"+fproxyport+"/"+ident.GetPublicKey()+messagebase+"|"+day+"|Message-"+idx+"?type=text/plain\"><i>"+subject+"</i></a>";
			stgot.Bind(0,uuid);
			stgot.Step();
			if(stgot.RowReturned())
			{
				std::string x="nie";
				stgot.ResultText(0,x);
				subject+=" (id="+x+")";
			}
			stgot.Reset();
		}
		stmsg.Reset();
		tblcontent+=subject;
		tblcontent+="</td><td>"+time+"</td>";
		//tblcontent+="<td class=\"smaller\">"+uuid+"</td>";

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
			tblcontent+="<td colspan=\"2\" style=\"text-align:left;\"><a href=\"?"+BuildQueryString(startrow-rowsperpage,identityidstr)+"\">"+m_trans->Get("web.page.insertedmessages.prevpage")+"</a></td>";
			cols+=2;
		}
		if(startrow+rowsperpage<rowscount)
		{
			while(cols<3)
			{
				tblcontent+="<td></td>";
				cols++;
			}
			tblcontent+="<td colspan=\"1\" style=\"text-align:left;\"><a href=\"?"+BuildQueryString(startrow+rowsperpage,identityidstr)+"\">"+m_trans->Get("web.page.insertedmessages.nextpage")+"</a></td>";
		}
		tblcontent+="</tr>";
	}

	tblcontent+="</table>";

	content="<h2>"+rowscountstr+" "+m_trans->Get("web.page.insertedmessages.msgssent");
	if (identityidstr!="") content+= " by a local user (<a href=\"?\">"+m_trans->Get("web.page.insertedmessages.showallusers")+"</a>)";
	content+="</h2>";

	content+=tblcontent;

	return content;
}
