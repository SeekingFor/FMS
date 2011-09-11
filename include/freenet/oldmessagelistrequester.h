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
	void PostHandleAllData(FCPv2::Message &message);
	void PostHandleGetFailed(FCPv2::Message &message);
	void PopulateRequestIDs(SQLite3DB::Transaction &trans, const long identityid, const Poco::DateTime &date);
	const std::string GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const std::string &id);

};

#endif	// _oldmessagelistrequester_
