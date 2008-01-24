#include "../include/message.h"
#include "../include/nntp/mime/Mime.h"
#include "../include/uuidgenerator.h"
#include "../include/stringfunctions.h"
#include "../include/freenet/messagexml.h"

#include <algorithm>

#ifdef XMEM
	#include <xmem.h>
#endif

Message::Message()
{
	Initialize();
}

Message::Message(const long messageid)
{
	Load(messageid);
}

const std::string Message::GetNNTPArticleID() const
{
	return "<"+m_messageuuid+">";
}

const std::string Message::GetNNTPBody() const
{
	return m_body;
}

const std::string Message::GetNNTPHeaders() const
{
	std::string rval("");

	rval+="From: "+m_fromname+"\r\n";
	rval+="Newsgroups: ";
	for(std::vector<std::string>::const_iterator i=m_boards.begin(); i!=m_boards.end(); i++)
	{
		if(i!=m_boards.begin())
		{
			rval+=",";
		}
		rval+=(*i);
	}
	rval+="\r\n";
	rval+="Subject: "+m_subject+"\r\n";
	// format time as  : Wdy, DD Mon YY HH:MM:SS TIMEZONE
	rval+="Date: "+m_datetime.Format("%a, %d %b %y %H:%M:%S -0000")+"\r\n";
	if(m_inreplyto.size()>0)
	{
		rval+="References: ";
		for(std::map<long,std::string>::const_reverse_iterator j=m_inreplyto.rbegin(); j!=m_inreplyto.rend(); j++)
		{
			if(j!=m_inreplyto.rend())
			{
				rval+=" ";
			}
			rval+="<"+(*j).second+">";
		}
		rval+="\r\n";
	}
	rval+="Followup-To: "+m_replyboardname+"\r\n";
	rval+="Path: freenet\r\n";
	rval+="Message-ID: "+GetNNTPArticleID()+"\r\n";
	rval+="Content-Type: text/plain; charset=UTF-8\r\n";

	return rval;
}

void Message::Initialize()
{
	m_messageid=-1;
	m_messageuuid="";
	m_subject="";
	m_body="";
	m_replyboardname="";
	m_datetime.Set();
	m_fromname="";
	m_boards.clear();
	m_inreplyto.clear();
}

const bool Message::Load(const long messageid, const long boardid)
{
	
	Initialize();

	std::string sql;
	
	sql="SELECT tblMessage.MessageID, MessageUUID, Subject, Body, tblBoard.BoardName, MessageDate, MessageTime, FromName FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID INNER JOIN tblBoard ON tblMessage.ReplyBoardID=tblBoard.BoardID WHERE tblMessage.MessageID=?";
	if(boardid!=-1)
	{
		sql+=" AND tblMessageBoard.BoardID=?";
	}
	sql+=";";

	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Bind(0,messageid);
	if(boardid!=-1)
	{
		st.Bind(1,boardid);
	}
	st.Step();

	if(st.RowReturned())
	{
		std::string tempdate;
		std::string temptime;
		int tempint=-1;
		st.ResultInt(0,tempint);
		m_messageid=tempint;
		st.ResultText(1,m_messageuuid);
		st.ResultText(2,m_subject);
		st.ResultText(3,m_body);
		st.ResultText(4,m_replyboardname);
		st.ResultText(5,tempdate);
		st.ResultText(6,temptime);
		m_datetime.Set(tempdate + " " + temptime);
		st.ResultText(7,m_fromname);
		st.Finalize();

		// get board list
		st=m_db->Prepare("SELECT tblBoard.BoardName FROM tblBoard INNER JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblMessageBoard.MessageID=?;");
		st.Bind(0,messageid);
		st.Step();
		while(st.RowReturned())
		{
			std::string tempval;
			st.ResultText(0,tempval);
			m_boards.push_back(tempval);
			st.Step();
		}
		st.Finalize();

		// get in reply to list
		st=m_db->Prepare("SELECT ReplyToMessageUUID, ReplyOrder FROM tblMessageReplyTo INNER JOIN tblMessage ON tblMessageReplyTo.MessageID=tblMessage.MessageID WHERE tblMessage.MessageID=?;");
		st.Bind(0,messageid);
		st.Step();
		while(st.RowReturned())
		{
			std::string tempval;
			int tempint;
			st.ResultText(0,tempval);
			st.ResultInt(1,tempint);
			m_inreplyto[tempint]=tempval;
			st.Step();
		}
		st.Finalize();

		return true;
	}
	else
	{
		return false;
	}

}

const bool Message::Load(const std::string &messageuuid)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT MessageID FROM tblMessage WHERE MessageUUID=?;");
	st.Bind(0,messageuuid);
	st.Step();

	if(st.RowReturned())
	{
		int messageid;
		st.ResultInt(0,messageid);

		return Load(messageid);
	}
	else
	{
		return false;
	}
}

const bool Message::LoadNext(const long messageid, const long boardid)
{
	std::string sql="SELECT MessageID FROM tblMessage WHERE MessageID>?";
	if(boardid!=-1)
	{
		sql+=" AND BoardID=?";
	}
	sql+=";";

	SQLite3DB::Statement st=m_db->Prepare(sql);

	st.Bind(0,messageid);
	if(boardid!=-1)
	{
		st.Bind(1,boardid);
	}
	st.Step();

	if(st.RowReturned())
	{
		int result;
		st.ResultInt(0,result);
		return Load(result,boardid);
	}
	else
	{
		return false;
	}
}

const bool Message::LoadPrevious(const long messageid, const long boardid)
{
	std::string sql="SELECT MessageID FROM tblMessage WHERE MessageID<?";
	if(boardid!=-1)
	{
		sql+=" AND BoardID=?";
	}
	sql+=";";

	SQLite3DB::Statement st=m_db->Prepare(sql);

	st.Bind(0,messageid);
	if(boardid!=-1)
	{
		st.Bind(1,boardid);
	}
	st.Step();

	if(st.RowReturned())
	{
		int result;
		st.ResultInt(0,result);
		return Load(result,boardid);
	}
	else
	{
		return false;
	}
}

const bool Message::ParseNNTPMessage(const std::string &nntpmessage)
{

	Initialize();

	UUIDGenerator uuid;
	CMimeMessage mime;
	mime.Load(nntpmessage.c_str(),nntpmessage.size());

	// get header info
	// date is always set to now regardless of what message has
	m_datetime.SetToGMTime();
	// messageuuid is always a unique id we generate regardless of message message-id
	m_messageuuid=uuid.Generate();
	// get from
	if(mime.GetFieldValue("From"))
	{
		m_fromname=mime.GetFieldValue("From");
		// strip off everything between () and <> and any whitespace
		std::string::size_type startpos=m_fromname.find("(");
		std::string::size_type endpos;
		if(startpos!=std::string::npos)
		{
			endpos=m_fromname.find(")",startpos);
			if(endpos!=std::string::npos)
			{
				m_fromname.erase(startpos,(endpos-startpos)+1);
			}
		}
		startpos=m_fromname.find("<");
		if(startpos!=std::string::npos)
		{
			endpos=m_fromname.find(">",startpos);
			if(endpos!=std::string::npos)
			{
				m_fromname.erase(startpos,(endpos-startpos)+1);
			}
		}
		m_fromname=StringFunctions::TrimWhitespace(m_fromname);
	}
	else
	{
		m_fromname="Anonymous";
	}
	// get boards posted to
	if(mime.GetFieldValue("Newsgroups"))
	{
		std::string temp=mime.GetFieldValue("Newsgroups");
		std::vector<std::string> parts;
		StringFunctions::SplitMultiple(temp,", \t",parts);
		for(std::vector<std::string>::iterator i=parts.begin(); i!=parts.end(); i++)
		{
			(*i)=StringFunctions::Replace((*i),"<","");
			(*i)=StringFunctions::Replace((*i),">","");
			(*i)=StringFunctions::TrimWhitespace((*i));
			if((*i)!="")
			{
				m_boards.push_back((*i));
			}
		}
	}
	// followup-to board - must be done after board vector populated
	if(mime.GetFieldValue("Followup-To"))
	{
		m_replyboardname=mime.GetFieldValue("Followup-To");
	}
	else
	{
		if(m_boards.size()>0)
		{
			m_replyboardname=m_boards[0];
		}
	}
	// subject
	if(mime.GetFieldValue("Subject"))
	{
		m_subject=mime.GetFieldValue("Subject");
	}
	else
	{
		m_subject="No Subject";
	}
	// references
	if(mime.GetFieldValue("References"))
	{
		std::string temp=mime.GetFieldValue("References");
		std::vector<std::string> parts;
		int count=0;
		StringFunctions::SplitMultiple(temp,", \t",parts);
		for(std::vector<std::string>::reverse_iterator i=parts.rbegin(); i!=parts.rend(); i++)
		{
			(*i)=StringFunctions::Replace((*i),"<","");
			(*i)=StringFunctions::Replace((*i),">","");
			(*i)=StringFunctions::TrimWhitespace((*i));
			if((*i)!="")
			{
				m_inreplyto[count++]=(*i);
			}
		}
	}

	CMimeBody::CBodyList mbl;
	mime.GetBodyPartList(mbl);

	// append all text parts of nntp message to body
	for(CMimeBody::CBodyList::iterator i=mbl.begin(); i!=mbl.end(); i++)
	{
		if((*i)->IsText() && (*i)->GetContent())
		{
			m_body+=(char *)(*i)->GetContent();
		}
	}

	return true;
}

void Message::StartFreenetInsert()
{
	MessageXML xml;
	int localidentityid=-1;

	xml.SetMessageID(m_messageuuid);
	xml.SetSubject(m_subject);
	xml.SetBody(m_body);
	xml.SetReplyBoard(m_replyboardname);
	xml.SetDate(m_datetime.Format("%Y-%m-%d"));
	xml.SetTime(m_datetime.Format("%H:%M:%S"));
	
	for(std::vector<std::string>::iterator i=m_boards.begin(); i!=m_boards.end(); i++)
	{
		xml.AddBoard((*i));
	}
	
	for(std::map<long,std::string>::iterator j=m_inreplyto.begin(); j!=m_inreplyto.end(); j++)
	{
		xml.AddInReplyTo((*j).first,(*j).second);
	}

	// find identity to insert with
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE Name=?;");
	st.Bind(0,m_fromname);
	st.Step();

	// couldn't find identity with this name - insert a new identity
	if(!st.RowReturned())
	{
		DateTime now;
		now.SetToGMTime();
		st=m_db->Prepare("INSERT INTO tblLocalIdentity(Name) VALUES(?);");
		st.Bind(0,m_fromname);
		st.Step(true);
		localidentityid=st.GetLastInsertRowID();
	}
	else
	{
		st.ResultInt(0,localidentityid);
	}

	st=m_db->Prepare("INSERT INTO tblMessageInserts(LocalIdentityID,MessageUUID,MessageXML) VALUES(?,?,?);");
	st.Bind(0,localidentityid);
	st.Bind(1,m_messageuuid);
	st.Bind(2,xml.GetXML());
	st.Step();

}
