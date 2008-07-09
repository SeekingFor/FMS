#include "../include/boardlist.h"

#ifdef XMEM
	#include <xmem.h>
#endif

void BoardList::Load()
{
	clear();
	
	int boardid=0;
	std::string boardname="";
	std::string boarddescription="";
	std::string dateadded="";
	std::string savereceivedstr="";
	bool savereceived=false;
	int highmessageid=0;
	int lowmessageid=0;
	int messagecount=0;
	std::string addedmethod="";

	SQLite3DB::Statement st=m_db->Prepare("SELECT tblBoard.BoardID, BoardName, BoardDescription, DateAdded, HighMessageID, LowMessageID, MessageCount, SaveReceivedMessages, AddedMethod FROM tblBoard LEFT JOIN vwBoardStats ON tblBoard.BoardID=vwBoardStats.BoardID ORDER BY BoardName COLLATE NOCASE;");
	st.Step();
	
	while(st.RowReturned())
	{
		st.ResultInt(0,boardid);
		st.ResultText(1,boardname);
		st.ResultText(2,boarddescription);
		st.ResultText(3,dateadded);
		st.ResultInt(4,highmessageid);
		st.ResultInt(5,lowmessageid);
		st.ResultInt(6,messagecount);
		st.ResultText(7,savereceivedstr);
		st.ResultText(8,addedmethod);

		if(savereceivedstr=="true")
		{
			savereceived=true;
		}		
		else
		{
			savereceived=false;
		}

		push_back(Board(boardid,boardname,boarddescription,dateadded,lowmessageid,highmessageid,messagecount,savereceived,addedmethod));
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
