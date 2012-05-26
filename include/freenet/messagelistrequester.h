#ifndef _messagelistrequester_
#define _messagelistrequester_

#include "imessagelistrequester.h"

class MessageListRequester:public IMessageListRequester<long>
{
public:
	MessageListRequester(SQLite3DB::DB *db);
	MessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	virtual void Initialize();
	virtual void PopulateIDList();
	const long GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const long &id);
};

#endif	// _messagelistrequester_
