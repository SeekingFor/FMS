#ifndef _messagelist_
#define _messagelist_

#include "message.h"
#include "ilogger.h"
#include "idatabase.h"

class MessageList:public std::vector<Message>,public ILogger,public IDatabase
{
public:
	MessageList(SQLite3DB::DB *db);
	
	/**
		\brief Loads all messages with an id between lowmessageid and highmessageid inclusive
	*/
	void LoadNNTPRange(const long lownntpmessageid, const long highnntpmessageid, const long boardid);
	
private:
	bool m_uniqueboardmessageids;
};

#endif	// _messagelist_
