#include "../include/messagethread.h"
#include "../include/stringfunctions.h"

#include <algorithm>

#ifdef XMEM
	#include <xmem.h>
#endif

void MessageThread::AddChildren(const long messageid, const long level, const long boardid)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessageReplyTo.MessageID, tblMessage1.Subject, tblMessage1.FromName, tblMessage1.MessageDate || ' ' || tblMessage1.MessageTime FROM tblMessage INNER JOIN tblMessageReplyTo ON tblMessage.MessageUUID=tblMessageReplyTo.ReplyToMessageUUID INNER JOIN tblMessage AS 'tblMessage1' ON tblMessageReplyTo.MessageID=tblMessage1.MessageID INNER JOIN tblMessageBoard ON tblMessage1.MessageID=tblMessageBoard.MessageID WHERE tblMessage.MessageID=? AND tblMessageBoard.BoardID=? AND tblMessageReplyTo.ReplyOrder=0 ORDER BY tblMessage1.MessageDate || ' ' || tblMessage1.MessageTime;");
	st.Bind(0,messageid);
	st.Bind(1,boardid);
	st.Step();
	while(st.RowReturned())
	{
		int childid=0;
		std::string subject="";
		std::string fromname="";
		std::string datetime="";
		st.ResultInt(0,childid);
		st.ResultText(1,subject);
		st.ResultText(2,fromname);
		st.ResultText(3,datetime);
		
		threadnode node;
		node.m_messageid=childid;
		node.m_level=level;
		node.m_subject=subject;
		node.m_fromname=fromname;
		node.m_date=datetime;
		m_nodes.push_back(node);
		
		AddChildren(childid,level+1,boardid);
		
		st.Step();
	}	
}

const MessageThread::threadnode MessageThread::GetOriginalMessageNode(const long messageid, const long boardid)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.Subject, tblMessage.FromName, tblMessage.MessageDate || ' ' || tblMessage.MessageTime FROM tblMessageReplyTo INNER JOIN tblMessage ON tblMessageReplyTo.ReplyToMessageUUID=tblMessage.MessageUUID INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessageReplyTo.ReplyOrder=0 AND tblMessageReplyTo.MessageID=? AND tblMessageBoard.BoardID=?;");
	st.Bind(0,messageid);
	st.Bind(1,boardid);
	st.Step();
	if(st.RowReturned())
	{
		int id=0;
		std::string subject="";
		std::string fromname="";
		std::string datetime="";
		st.ResultInt(0,id);
		st.ResultText(1,subject);
		st.ResultText(2,fromname);
		st.ResultText(3,datetime);

		threadnode node;
		node.m_messageid=id;
		node.m_level=0;
		node.m_subject=subject;
		node.m_fromname=fromname;
		node.m_date=datetime;

		return GetOriginalMessageNode(node.m_messageid,boardid);
	}
	else
	{
		threadnode node;
		node.m_messageid=-1;
		node.m_level=0;
		node.m_subject="";
		node.m_fromname="";
		node.m_date="";

		SQLite3DB::Statement st2=m_db->Prepare("SELECT Subject, FromName, MessageDate || ' ' || MessageTime FROM tblMessage WHERE MessageID=?;");
		st2.Bind(0,messageid);
		st2.Step();

		if(st2.RowReturned())
		{
			node.m_messageid=messageid;
			st2.ResultText(0,node.m_subject);
			st2.ResultText(1,node.m_fromname);
			st2.ResultText(2,node.m_date);
		}

		return node;
	}
}

const bool MessageThread::Load(const long messageid, const long boardid, const bool bydate)
{
	threadnode originalmessagenode=GetOriginalMessageNode(messageid,boardid);
	
	if(originalmessagenode.m_messageid>=0)
	{
		m_nodes.push_back(originalmessagenode);
		
		AddChildren(originalmessagenode.m_messageid,1,boardid);

		if(bydate==true)
		{
			std::sort(m_nodes.begin(),m_nodes.end(),datecompare());
		}

		return true;
	}
	else
	{
		return false;
	}
}

const bool MessageThread::Load(const std::string &messageidstr, const long boardid, const bool bydate)
{
	long messageid=0;
	StringFunctions::Convert(messageidstr,messageid);
	return Load(messageid,boardid,bydate);
}
