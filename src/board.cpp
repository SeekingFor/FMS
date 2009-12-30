#include "../include/board.h"
#include "../include/stringfunctions.h"
#include "../include/option.h"

#include <Poco/DateTimeParser.h>

#ifdef XMEM
	#include <xmem.h>
#endif

Board::Board(SQLite3DB::DB *db):IDatabase(db)
{
	Option option(db);
	option.GetBool("UniqueBoardMessageIDs",m_uniqueboardmessageids);

	m_boardid=-1;
	m_boardname="";
	m_boarddescription="";
	m_datecreated.assign(1970,1,1);
	m_lownntpmessageid=0;
	m_highnntpmessageid=0;
	m_messagecount=0;
	m_savereceivedmessages=true;
	m_addedmethod="";
}

Board::Board(SQLite3DB::DB *db, const long boardid):IDatabase(db)
{
	Option option(db);
	option.GetBool("UniqueBoardMessageIDs",m_uniqueboardmessageids);
	Load(boardid);	
}

Board::Board(SQLite3DB::DB *db, const std::string &boardname):IDatabase(db)
{
	Option option(db);
	option.GetBool("UniqueBoardMessageIDs",m_uniqueboardmessageids);
	Load(boardname);
}

Board::Board(SQLite3DB::DB *db, const long boardid, const std::string &boardname, const std::string &boarddescription, const std::string datecreated, const long lownntpmessageid, const long highnntpmessageid, const long messagecount, const bool savereceivedmessages, const std::string &addedmethod):IDatabase(db)
{
	Option option(db);
	option.GetBool("UniqueBoardMessageIDs",m_uniqueboardmessageids);
	m_boardid=boardid;
	m_boardname=boardname;
	m_boarddescription=boarddescription;
	m_lownntpmessageid=lownntpmessageid;
	m_highnntpmessageid=highnntpmessageid;
	m_messagecount=messagecount;
	m_savereceivedmessages=savereceivedmessages;
	m_addedmethod=addedmethod;

	SetDateFromString(datecreated);

}


const bool Board::Load(const long boardid)
{
	// clear current values
	m_boardid=-1;
	m_boardname="";
	m_boarddescription="";
	m_datecreated.assign(1970,1,1);
	m_lownntpmessageid=0;
	m_highnntpmessageid=0;
	m_messagecount=0;
	m_addedmethod="";

	// Optimize query by not using vwBoardStats
	//SQLite3DB::Statement st=m_db->Prepare("SELECT BoardName, BoardDescription, DateAdded, HighMessageID, LowMessageID, MessageCount, SaveReceivedMessages, AddedMethod FROM tblBoard LEFT JOIN vwBoardStats ON tblBoard.BoardID=vwBoardStats.BoardID WHERE tblBoard.BoardID=?;");
	SQLite3DB::Statement st;
	if(m_uniqueboardmessageids==true)
	{
		st=m_db->Prepare("SELECT BoardName, BoardDescription, DateAdded, IFNULL(MAX(BoardMessageID),0) AS HighMessageID, IFNULL(MIN(BoardMessageID),0) AS LowMessageID, COUNT(MessageID) AS MessageCount, SaveReceivedMessages, AddedMethod FROM tblBoard LEFT JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblBoard.BoardID=? AND (MessageID IS NULL OR MessageID>=0);");
	}
	else
	{
		st=m_db->Prepare("SELECT BoardName, BoardDescription, DateAdded, IFNULL(MAX(MessageID),'0') AS HighMessageID, IFNULL(MIN(MessageID),'0') AS LowMessageID, COUNT(MessageID) AS MessageCount, SaveReceivedMessages, AddedMethod FROM tblBoard LEFT JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblBoard.BoardID=? AND (MessageID IS NULL OR MessageID>=0);");
	}
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

		SetDateFromString(tempstr);

		tempint=0;
		st.ResultInt(3,tempint);
		m_highnntpmessageid=tempint;
		tempint=0;
		st.ResultInt(4,tempint);
		m_lownntpmessageid=tempint;
		tempint=0;
		st.ResultInt(5,tempint);
		m_messagecount=tempint;
		st.ResultText(6,tempstr);
		if(tempstr=="true")
		{
			m_savereceivedmessages=true;
		}
		else
		{
			m_savereceivedmessages=false;
		}
		st.ResultText(7,m_addedmethod);

		return true;
	}
	else
	{
		return false;
	}
}

const bool Board::Load(const std::string &boardname)		// same as loading form boardid - but using name
{

	// clear current values
	m_boardid=-1;
	m_boardname="";
	m_boarddescription="";
	m_datecreated.assign(1970,1,1);
	m_lownntpmessageid=0;
	m_highnntpmessageid=0;
	m_messagecount=0;
	int tempint=-1;
	m_addedmethod="";

	// Optimize query by not using vwBoardStats
	//SQLite3DB::Statement st=m_db->Prepare("SELECT BoardName, BoardDescription, DateAdded, HighMessageID, LowMessageID, MessageCount, SaveReceivedMessages, tblBoard.BoardID, AddedMethod FROM tblBoard LEFT JOIN vwBoardStats ON tblBoard.BoardID=vwBoardStats.BoardID WHERE tblBoard.BoardName=?;");
	SQLite3DB::Statement st;
	if(m_uniqueboardmessageids==true)
	{
		st=m_db->Prepare("SELECT BoardName, BoardDescription, DateAdded, IFNULL(MAX(BoardMessageID),0) AS HighMessageID, IFNULL(MIN(BoardMessageID),0) AS LowMessageID, COUNT(MessageID) AS MessageCount, SaveReceivedMessages, tblBoard.BoardID, AddedMethod FROM tblBoard LEFT JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblBoard.BoardName=? AND (MessageID IS NULL OR MessageID>=0);");
	}
	else
	{
		st=m_db->Prepare("SELECT BoardName, BoardDescription, DateAdded, IFNULL(MAX(MessageID),'0') AS HighMessageID, IFNULL(MIN(MessageID),'0') AS LowMessageID, COUNT(MessageID) AS MessageCount, SaveReceivedMessages, tblBoard.BoardID, AddedMethod FROM tblBoard LEFT JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblBoard.BoardName=? AND (MessageID IS NULL OR MessageID>=0);");
	}
	st.Bind(0,boardname);
	st.Step();

	if(st.RowReturned())
	{
		int tempint;
		std::string tempstr;
		std::vector<std::string> dateparts;

		st.ResultText(0,m_boardname);
		st.ResultText(1,m_boarddescription);
		st.ResultText(2,tempstr);
		st.ResultInt(7,tempint);	// boardid
		m_boardid=tempint;

		// queries always return empty row even for no matches
		if(m_boardname!=boardname)
		{
			return false;
		}

		SetDateFromString(tempstr);

		tempint=0;
		st.ResultInt(3,tempint);
		m_highnntpmessageid=tempint;
		tempint=0;
		st.ResultInt(4,tempint);
		m_lownntpmessageid=tempint;
		tempint=0;
		st.ResultInt(5,tempint);
		m_messagecount=tempint;
		st.ResultText(6,tempstr);
		if(tempstr=="true")
		{
			m_savereceivedmessages=true;
		}
		else
		{
			m_savereceivedmessages=false;
		}
		st.ResultText(8,m_addedmethod);

		return true;
	}
	else
	{
		return false;
	}

}

void Board::SetDateFromString(const std::string &datestring)
{
	int tzdiff=0;
	if(Poco::DateTimeParser::tryParse(datestring,m_datecreated,tzdiff)==false)
	{
		m_log->error("Board::SetDateFromString could not parse date "+datestring);
	}
}

void Board::SetSaveReceivedMessages(const bool savereceivedmessages)
{
	if(m_savereceivedmessages!=savereceivedmessages)
	{
		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblBoard SET SaveReceivedMessages=? WHERE BoardID=?;");
		if(savereceivedmessages==true)
		{
			st.Bind(0,"true");
		}
		else
		{
			st.Bind(0,"false");
		}
		st.Bind(1,m_boardid);
		st.Step();
		m_savereceivedmessages=savereceivedmessages;
	}
}
