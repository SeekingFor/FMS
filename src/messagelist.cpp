#include "../include/messagelist.h"

void MessageList::LoadRange(const long lowmessageid, const long highmessageid, const long boardid)
{
	std::string sql;

	sql="SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessage.MessageID>=? AND tblMessage.MessageID<=?";
	if(boardid!=-1)
	{
		sql+=" AND tblMessageBoard.BoardID=?";
	}
	sql+=" ORDER BY tblMessage.MessageID;";

	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Bind(0,lowmessageid);
	st.Bind(1,highmessageid);
	if(boardid!=-1)
	{
		st.Bind(2,boardid);
	}
	st.Step();

	// clear existing messages from the list
	clear();

	while(st.RowReturned())
	{
		int result;
		st.ResultInt(0,result);
		push_back(Message(m_db,result));
		st.Step();
	}

}
