#include "../../include/http/forumviewstate.h"

#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>

ForumViewState::ForumViewState(SQLite3DB::DB *db):IDatabase(db),m_viewstateid(""),m_localidentityid(0)
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
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tmpForumViewState WHERE ForumViewStateID=?;");
	st.Bind(0,viewstateid);
	st.Step();
	if(st.RowReturned())
	{
		m_viewstateid=viewstateid;
		st.ResultInt(0,m_localidentityid);
		return true;
	}
	else
	{
		m_viewstateid="";
		m_localidentityid=0;
		return false;
	}
}

void ForumViewState::SetLocalIdentityID(const int localidentityid)
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tmpForumViewState SET LocalIdentityID=? WHERE ForumViewStateID=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,m_viewstateid);
	st.Step();
	m_localidentityid=localidentityid;
}
