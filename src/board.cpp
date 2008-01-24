#include "../include/board.h"
#include "../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

Board::Board()
{
	m_boardid=-1;
	m_boardname="";
	m_boarddescription="";
	m_datecreated.Set(1970,1,1);
	m_lowmessageid=0;
	m_highmessageid=0;
	m_messagecount=0;
}

Board::Board(const long boardid)
{
	Load(boardid);	
}

Board::Board(const std::string &boardname)
{
	Load(boardname);
}

const bool Board::Load(const long boardid)
{
	// clear current values
	m_boardid=-1;
	m_boardname="";
	m_boarddescription="";
	m_datecreated.Set(1970,1,1);
	m_lowmessageid=0;
	m_highmessageid=0;
	m_messagecount=0;

	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardName, BoardDescription, DateAdded FROM tblBoard WHERE BoardID=?;");
	st.Bind(0,boardid);
	st.Step();

	if(st.RowReturned())
	{
		int tempint;
		std::string tempstr;
		std::vector<std::string> dateparts;

		m_boardid=boardid;
		st.ResultText(0,m_boardname);
		st.ResultText(1,m_boarddescription);
		st.ResultText(2,tempstr);

		// break out date created  - date should be in format yyyy-mm-dd HH:MM:SS, so we split on "-", " " (space), and ":"
		StringFunctions::SplitMultiple(tempstr,"- :",dateparts);
		if(dateparts.size()>0)
		{
			StringFunctions::Convert(dateparts[0],tempint);
			m_datecreated.SetYear(tempint);
		}
		if(dateparts.size()>1)
		{
			StringFunctions::Convert(dateparts[1],tempint);
			m_datecreated.SetMonth(tempint);
		}
		if(dateparts.size()>2)
		{
			StringFunctions::Convert(dateparts[2],tempint);
			m_datecreated.SetDay(tempint);
		}
		if(dateparts.size()>3)
		{
			StringFunctions::Convert(dateparts[3],tempint);
			m_datecreated.SetHour(tempint);
		}
		if(dateparts.size()>4)
		{
			StringFunctions::Convert(dateparts[4],tempint);
			m_datecreated.SetMinute(tempint);
		}
		if(dateparts.size()>5)
		{
			StringFunctions::Convert(dateparts[5],tempint);
			m_datecreated.SetSecond(tempint);
		}

		// get max and min ids and message count in this board
		SQLite3DB::Statement bounds=m_db->Prepare("SELECT HighMessageID, LowMessageID, MessageCount FROM vwBoardStats WHERE BoardID=?;");
		bounds.Bind(0,boardid);
		bounds.Step();

		if(bounds.RowReturned())
		{
			int tempint;
			bounds.ResultInt(0,tempint);
			m_highmessageid=tempint;
			bounds.ResultInt(1,tempint);
			m_lowmessageid=tempint;
			bounds.ResultInt(2,tempint);
			m_messagecount=tempint;
		}

		return true;
	}
	else
	{
		return false;
	}
}

const bool Board::Load(const std::string &boardname)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID FROM tblBoard WHERE BoardName=?;");
	st.Bind(0,boardname);
	st.Step();
	if(st.RowReturned())
	{
		int tempint;
		st.ResultInt(0,tempint);
		return Load(tempint);
	}
	else
	{
		return false;
	}
}
