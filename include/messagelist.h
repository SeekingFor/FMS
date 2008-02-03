#ifndef _messagelist_
#define _messagelist_

#include "message.h"
#include "ilogger.h"
#include "idatabase.h"

class MessageList:public std::vector<Message>,public ILogger,public IDatabase
{
public:
	
	/**
		\brief Loads all messages with an id between lowmessageid and highmessageid inclusive
	*/
	void LoadRange(const long lowmessageid, const long highmessageid, const long boardid=-1);
	
private:

};

#endif	// _messagelist_
