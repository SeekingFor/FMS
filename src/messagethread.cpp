#include "../include/messagethread.h"
#include "../include/stringfunctions.h"

#include <algorithm>

#ifdef XMEM
	#include <xmem.h>
#endif

const bool MessageThread::Added(const long messageid) const
{
	for(std::vector<threadnode>::const_iterator i=m_nodes.begin(); i!=m_nodes.end(); i++)
	{
		if((*i).m_messageid==messageid)
		{
			return true;
		}
	}
	return false;
}

void MessageThread::AddChildren(const long messageid, const long level, const long boardid, const unsigned int currentdepth)
{
	int replyorder=0;
	if(currentdepth<1000)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessageReplyTo.MessageID, tblMessage1.Subject, tblMessage1.FromName, tblMessage1.MessageDate || ' ' || tblMessage1.MessageTime FROM tblMessage INNER JOIN tblMessageReplyTo ON tblMessage.MessageUUID=tblMessageReplyTo.ReplyToMessageUUID INNER JOIN tblMessage AS 'tblMessage1' ON tblMessageReplyTo.MessageID=tblMessage1.MessageID INNER JOIN tblMessageBoard ON tblMessage1.MessageID=tblMessageBoard.MessageID WHERE tblMessage.MessageID=? AND tblMessage1.MessageID!=? AND tblMessageBoard.BoardID=? AND tblMessageReplyTo.ReplyOrder=? ORDER BY tblMessage1.MessageDate || ' ' || tblMessage1.MessageTime;");
		for(replyorder=0; replyorder<5; replyorder++)
		{
			st.Bind(0,messageid);
			st.Bind(1,messageid);
			st.Bind(2,boardid);
			st.Bind(3,replyorder);
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

				if(childid!=messageid)
				{
					if(Added(childid)==false)
					{
						threadnode node;
						node.m_messageid=childid;
						node.m_level=level;
						node.m_subject=subject;
						node.m_fromname=fromname;
						node.m_date=datetime;
						m_nodes.push_back(node);

						AddChildren(childid,level+1,boardid,currentdepth+1);
					}
				}
				else
				{
					std::string mid("");
					std::string bid("");
					StringFunctions::Convert(messageid,mid);
					StringFunctions::Convert(boardid,bid);
					m_log->debug("MessageThread::AddChildren messageid "+mid+" tried to add itself as child in board "+bid);
				}
				
				st.Step();
			}
			st.Reset();
		}
	}
}

void MessageThread::AddChildren(const std::string &messageuuid, const long boardid)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.Subject, tblMessage.FromName, tblMessage.MessageDate || ' ' || tblMessage.MessageTime FROM tblMessageReplyTo INNER JOIN tblMessage ON tblMessageReplyTo.MessageID=tblMessage.MessageID WHERE ReplyToMessageUUID=?;");
	st.Bind(0,messageuuid);
	st.Step();
	while(st.RowReturned())
	{
		int messageid=0;
		std::string subject="";
		std::string fromname="";
		std::string datetime="";

		st.ResultInt(0,messageid);
		st.ResultText(1,subject);
		st.ResultText(2,fromname);
		st.ResultText(3,datetime);

		if(Added(messageid)==false)
		{
			threadnode node;
			node.m_messageid=messageid;
			node.m_level=1;
			node.m_subject=subject;
			node.m_fromname=fromname;
			node.m_date=datetime;
			m_nodes.push_back(node);
		}

		st.Step();
	}
}

const MessageThread::threadnode MessageThread::GetOriginalMessageNode(const long messageid, const long boardid, const unsigned int currentdepth)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.Subject, tblMessage.FromName, tblMessage.MessageDate || ' ' || tblMessage.MessageTime FROM tblMessageReplyTo INNER JOIN tblMessage ON tblMessageReplyTo.ReplyToMessageUUID=tblMessage.MessageUUID INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessageReplyTo.MessageID=? AND tblMessageBoard.BoardID=? ORDER BY tblMessageReplyTo.ReplyOrder DESC LIMIT 0,1;");
	st.Bind(0,messageid);
	st.Bind(1,boardid);
	st.Step();
	if(st.RowReturned() && currentdepth<1000)
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

		return GetOriginalMessageNode(node.m_messageid,boardid,currentdepth+1);
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
	threadnode originalmessagenode=GetOriginalMessageNode(messageid,boardid,0);
	
	if(originalmessagenode.m_messageid>=0)
	{
		m_nodes.push_back(originalmessagenode);

		// we couldn't get a parent message, i.e. this message is the most top level one we know about
		// Add single level of children for Sone messages if the top level message isn't known yet
		if(originalmessagenode.m_messageid==messageid)
		{
			// thread Sones where the initial message is missing.  All child Sones are replies to the missing initial message
			SQLite3DB::Statement st=m_db->Prepare("SELECT ReplyToMessageUUID FROM tblMessageReplyTo WHERE MessageID=?;");
			st.Bind(0,messageid);
			st.Step();
			if(st.RowReturned())
			{
				std::string messageuuid("");
				st.ResultText(0,messageuuid);
				AddChildren(messageuuid,boardid);
			}
		}
		
		// add all descendants of the original parent message
		AddChildren(originalmessagenode.m_messageid,1,boardid,0);

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
