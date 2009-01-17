#ifndef _trustlistrequester_
#define _trustlistrequester_

#include "iindexrequester.h"

class TrustListRequester:public IIndexRequester<long>//public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	TrustListRequester(SQLite3DB::DB *db);
	TrustListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();				// clear and re-populate m_ids with identities we want to query
	void StartRequest(const long &identityid);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

};

#endif	// _trustlistrequester_
