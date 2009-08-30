#ifndef _oldmessagelistrequester_
#define _oldmessagelistrequester_

#include "imessagelistrequester.h"

class OldMessageListRequester:public IMessageListRequester<std::string>
{
public:
	OldMessageListRequester(SQLite3DB::DB *db);
	OldMessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

protected:
	void Initialize();
	void PopulateIDList();
	const std::string GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const std::string &id);

};

#endif	// _oldmessagelistrequester_
