#include "../include/boardlist.h"

#ifdef XMEM
	#include <xmem.h>
#endif

void BoardList::Load()
{
	clear();
	
	int tempint;
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID FROM tblBoard ORDER BY BoardName;");
	st.Step();
	
	while(st.RowReturned())
	{
		st.ResultInt(0,tempint);
		push_back(Board(tempint));
		st.Step();
	}
}

void BoardList::LoadNew(const std::string &date)
{
	clear();

	int tempint;
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID FROM tblBoard WHERE DateAdded>? ORDER BY BoardName;");
	st.Bind(0,date);
	st.Step();

	while(st.RowReturned())
	{
		st.ResultInt(0,tempint);
		push_back(Board(tempint));
		st.Step();
	}
}
