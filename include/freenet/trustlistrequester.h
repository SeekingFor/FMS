#ifndef _trustlistrequester_
#define _trustlistrequester_

#include "iindexrequester.h"

class TrustListRequester:public IIndexRequester<long>
{
public:
	TrustListRequester(SQLite3DB::DB *db);
	TrustListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();				// clear and re-populate m_ids with identities we want to query
	const long GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const long &identityid);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

	long m_maxsize;

};

#endif	// _trustlistrequester_
