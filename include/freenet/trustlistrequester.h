#ifndef _trustlistrequester_
#define _trustlistrequester_

#include "iindexrequester.h"

class TrustListRequester:public IIndexRequester<long>//public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	TrustListRequester();
	TrustListRequester(FCPv2 *fcp);

private:
	void Initialize();
	void PopulateIDList();				// clear and re-populate m_ids with identities we want to query
	void StartRequest(const long &identityid);
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);

};

#endif	// _trustlistrequester_
