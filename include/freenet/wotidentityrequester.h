#ifndef _wotidentityrequester_
#define _wotidentityrequester_

#include "iindexrequester.h"

class WOTIdentityRequester:public IIndexRequester<std::pair<long,long> >
{
public:
	WOTIdentityRequester(SQLite3DB::DB *db);
	WOTIdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();
	const std::pair<long,long> GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const std::pair<long,long> &inputpair);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

};

#endif	// _wotidentityrequester_
