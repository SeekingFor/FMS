#ifndef _board_
#define _board_

#include "ilogger.h"
#include "idatabase.h"

#include <string>
#include <Poco/DateTime.h>

class Board:public ILogger,public IDatabase
{
public:
	Board(SQLite3DB::DB *db);
	Board(SQLite3DB::DB *db, const long boardid);
	Board(SQLite3DB::DB *db, const std::string &boardname);
	Board(SQLite3DB::DB *db, const long boardid, const std::string &boardname, const std::string &boarddescription, const std::string datecreated, const long lownntpmessageid, const long highnntpmessageid, const long messagecount, const bool savereceivedmessages, const std::string &addedmethod);

	const bool Load(const long boardid);
	const bool Load(const std::string &boardname);

	const long GetBoardID()	const				{ return m_boardid; }
	std::string GetBoardName() const			{ return m_boardname; }
	std::string GetBoardDescription() const		{ return m_boarddescription; }
	Poco::DateTime GetDateCreated() const		{ return m_datecreated; }
	const long GetLowNNTPMessageID() const		{ return m_lownntpmessageid; }
	const long GetHighNNTPMessageID() const		{ return m_highnntpmessageid; }
	const long GetMessageCount() const			{ return m_messagecount; }
	const bool GetSaveReceivedMessages() const	{ return m_savereceivedmessages; }
	std::string GetAddedMethod() const			{ return m_addedmethod; }

	void SetSaveReceivedMessages(const bool savereceivedmessages);

	static std::string FixBoardName(const std::string &boardname);

private:
	void SetDateFromString(const std::string &datestring);

	bool m_uniqueboardmessageids;
	long m_boardid;
	std::string m_boardname;
	std::string m_boarddescription;
	Poco::DateTime m_datecreated;
	long m_lownntpmessageid;		// lowest id of all message currently in this board
	long m_highnntpmessageid;		// highest id of all message currently in this board
	long m_messagecount;		// number of messages in this board
	bool m_savereceivedmessages;
	std::string m_addedmethod;
};

#endif	// _board_
