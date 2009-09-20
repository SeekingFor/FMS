#include "../include/messagelist.h"
#include "../include/option.h"

MessageList::MessageList(SQLite3DB::DB *db):IDatabase(db)
{
	Option option(db);
	option.GetBool("UniqueBoardMessageIDs",m_uniqueboardmessageids);
}

void MessageList::LoadNNTPRange(const long lownntpmessageid, const long highnntpmessageid, const long boardid)
{
	std::string sql;

	if(m_uniqueboardmessageids==true)
	{
		sql="SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessageBoard.BoardMessageID BETWEEN ? AND ?";
	}
	else
	{
		sql="SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessage.MessageID>=? AND tblMessage.MessageID<=?";
	}
	if(boardid!=-1)
	{
		sql+=" AND tblMessageBoard.BoardID=?";
	}
	sql+=" ORDER BY tblMessage.MessageID;";

	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Bind(0,lownntpmessageid);
	st.Bind(1,highnntpmessageid);
	if(boardid!=-1)
	{
		st.Bind(2,boardid);
	}
	st.Step();

	// clear existing messages from the list
	clear();

	while(st.RowReturned())
	{
		int dbmessageid;
		st.ResultInt(0,dbmessageid);
		push_back(Message(m_db,dbmessageid,boardid));
		st.Step();
	}

}
