#ifndef _inactive_identity_requester_
#define _inactive_identity_requester_

#include "identityrequester.h"

class InactiveIdentityRequester:public IdentityRequester
{
public:
	InactiveIdentityRequester(SQLite3DB::DB *db);
	InactiveIdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();				// clear and re-populate m_ids with identities we want to query

};

#endif	// _inactive_identity_requester_
