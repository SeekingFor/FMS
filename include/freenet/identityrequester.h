#ifndef _identity_requester_
#define _identity_requester_

#include "iindexrequester.h"

#include <utility>

class IdentityRequester:public IIndexRequester<std::pair<long,long> >
{
public:
	IdentityRequester(SQLite3DB::DB *db);
	IdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	virtual void Initialize()=0;
	virtual void PopulateIDList()=0;				// clear and re-populate m_ids with identities we want to query
	const std::pair<long,long> GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const std::pair<long,long> &inputpair);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

};

#endif	// _identity_requester_
