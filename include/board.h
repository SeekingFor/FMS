#ifndef _board_
#define _board_

#include "datetime.h"
#include "ilogger.h"
#include "idatabase.h"

#include <string>

class Board:public ILogger,public IDatabase
{
public:
	Board();
	Board(const long boardid);
	Board(const std::string &boardname);

	const bool Load(const long boardid);
	const bool Load(const std::string &boardname);

	const long GetBoardID()	const { return m_boardid; }
	std::string GetBoardName() const { return m_boardname; }
	std::string GetBoardDescription() const { return m_boarddescription; }
	DateTime GetDateCreated() const { return m_datecreated; }
	const long GetLowMessageID() const { return m_lowmessageid; }
	const long GetHighMessageID() const { return m_highmessageid; }
	const long GetMessageCount() const { return m_messagecount; }

private:
	long m_boardid;
	std::string m_boardname;
	std::string m_boarddescription;
	DateTime m_datecreated;
	long m_lowmessageid;		// lowest id of all message currently in this board
	long m_highmessageid;		// highest id of all message currently in this board
	long m_messagecount;		// number of messages in this board
};

#endif	// _board_
