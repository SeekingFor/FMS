#include "../../include/http/forumviewstate.h"

#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>

ForumViewState::ForumViewState(SQLite3DB::DB *db):IDatabase(db),m_viewstateid(""),m_localidentityid(0),m_boardid(0),m_page(0),m_threadid(0),m_messageid(0),m_replytomessageid(0)
{

}

const std::string ForumViewState::CreateViewState()
{
	Poco::UUIDGenerator uuidgen;
	Poco::UUID uuid;

	try
	{
		uuid=uuidgen.createRandom();
		m_viewstateid=uuid.toString();
		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tmpForumViewState(ForumViewStateID) VALUES(?);");
		st.Bind(0,m_viewstateid);
		st.Step();
	}
	catch(...)
	{
		m_viewstateid="";
	}

	return m_viewstateid;
}

const bool ForumViewState::LoadViewState(const std::string &viewstateid)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, BoardID, Page, ThreadID, MessageID, ReplyToMessageID FROM tmpForumViewState WHERE ForumViewStateID=?;");
	st.Bind(0,viewstateid);
	st.Step();
	if(st.RowReturned())
	{
		m_viewstateid=viewstateid;
		st.ResultInt(0,m_localidentityid);
		st.ResultInt(1,m_boardid);
		st.ResultInt(2,m_page);
		st.ResultInt(3,m_threadid);
		st.ResultInt(4,m_messageid);
		st.ResultInt(5,m_replytomessageid);
		return true;
	}
	else
	{
		m_viewstateid="";
		m_localidentityid=0;
		m_boardid=0;
		m_page=0;
		m_threadid=0;
		m_messageid=0;
		m_replytomessageid=0;
		return false;
	}
}

void ForumViewState::SetBoardID(const int boardid)
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET BoardID=? WHERE ForumViewStateID=?;");
	st.Bind(0,boardid);
	st.Bind(1,m_viewstateid);
	st.Step();
	m_boardid=boardid;
}

void ForumViewState::UnsetBoardID()
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET BoardID=NULL WHERE ForumViewStateID=?;");
	st.Bind(0,m_viewstateid);
	st.Step();
	m_boardid=0;
}

void ForumViewState::SetLocalIdentityID(const int localidentityid)
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET LocalIdentityID=? WHERE ForumViewStateID=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,m_viewstateid);
	st.Step();
	m_localidentityid=localidentityid;
}

void ForumViewState::UnsetLocalIdentityID()
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET LocalIdentityID=NULL WHERE ForumViewStateID=?;");
	st.Bind(0,m_viewstateid);
	st.Step();
	m_localidentityid=0;
}

void ForumViewState::SetPage(const int page)
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET Page=? WHERE ForumViewStateID=?;");
	st.Bind(0,page);
	st.Bind(1,m_viewstateid);
	st.Step();
	m_page=page;
}

void ForumViewState::UnsetPage()
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET Page=NULL WHERE ForumViewStateID=?;");
	st.Bind(0,m_viewstateid);
	st.Step();
	m_page=0;
}

void ForumViewState::SetThreadID(const int threadid)
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET ThreadID=? WHERE ForumViewStateID=?;");
	st.Bind(0,threadid);
	st.Bind(1,m_viewstateid);
	st.Step();
	m_threadid=threadid;
}

void ForumViewState::UnsetThreadID()
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET ThreadID=NULL WHERE ForumViewStateID=?;");
	st.Bind(0,m_viewstateid);
	st.Step();
	m_threadid=0;
}

void ForumViewState::SetMessageID(const int messageid)
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET MessageID=? WHERE ForumViewStateID=?;");
	st.Bind(0,messageid);
	st.Bind(1,m_viewstateid);
	st.Step();
	m_messageid=messageid;
}

void ForumViewState::UnsetMessageID()
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET MessageID=NULL WHERE ForumViewStateID=?;");
	st.Bind(0,m_viewstateid);
	st.Step();
	m_messageid=0;
}

void ForumViewState::SetReplyToMessageID(const int replytomessageid)
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET ReplyToMessageID=? WHERE ForumViewStateID=?;");
	st.Bind(0,replytomessageid);
	st.Bind(1,m_viewstateid);
	st.Step();
	m_replytomessageid=replytomessageid;
}

void ForumViewState::UnsetReplyToMessageID()
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET ReplyToMessageID=NULL WHERE ForumViewStateID=?;");
	st.Bind(0,m_viewstateid);
	st.Step();
	m_replytomessageid=0;
}