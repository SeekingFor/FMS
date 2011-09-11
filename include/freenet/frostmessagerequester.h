#ifndef _frost_message_requester_
#define _frost_message_requester_

#include "iindexrequester.h"

class FrostMessageRequester:public IIndexRequester<std::string>
{
public:
	FrostMessageRequester(SQLite3DB::DB *db);
	FrostMessageRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();
	void PopulateBoardRequestIDs(SQLite3DB::Transaction &trans, const long boardid, const Poco::DateTime &date);
	const std::string GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const std::string &id);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

	std::string m_boardprefix;
	std::string m_frostmessagebase;
	int m_maxdaysbackward;
	int m_maxindexesforward;
	bool m_saveanonymous;

};

#endif	// _frost_message_requester_
