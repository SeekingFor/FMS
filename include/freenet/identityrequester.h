#ifndef _identity_requester_
#define _identity_requester_

#include "iindexrequester.h"

class IdentityRequester:public IIndexRequester<long>
{
public:
	IdentityRequester(SQLite3DB::DB *db);
	IdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	virtual void Initialize();
	virtual void PopulateIDList();				// clear and re-populate m_ids with identities we want to query
	void StartRequest(const long &identityid);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

};

#endif	// _identity_requester_
