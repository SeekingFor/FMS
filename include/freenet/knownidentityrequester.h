#ifndef _known_identity_requester_
#define _known_identity_requester_

#include "identityrequester.h"

class KnownIdentityRequester:public IdentityRequester
{
public:
	KnownIdentityRequester(SQLite3DB::DB *db);
	KnownIdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();				// clear and re-populate m_ids with identities we want to query

};

#endif	// _known_identity_requester_
