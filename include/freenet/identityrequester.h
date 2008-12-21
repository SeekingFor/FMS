#ifndef _identity_requester_
#define _identity_requester_

#include "iindexrequester.h"

class IdentityRequester:public IIndexRequester<long>
{
public:
	IdentityRequester();
	IdentityRequester(FCPv2 *fcp);

private:
	virtual void Initialize();
	virtual void PopulateIDList();				// clear and re-populate m_ids with identities we want to query
	void StartRequest(const long &identityid);
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);

};

#endif	// _identity_requester_
