#ifndef _identityintroduction_requester_
#define _identityintroduction_requester_

#include "iindexrequester.h"

#include <Poco/DateTime.h>

class IdentityIntroductionRequester:public IIndexRequester<std::string>
{
public:
	IdentityIntroductionRequester(SQLite3DB::DB *db);
	IdentityIntroductionRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	const std::string GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const std::string &UUID);
	void PopulateIDList();
	const bool HandleGetFailed(FCPv2::Message &message);
	const bool HandleAllData(FCPv2::Message &message);

};

#endif	// _identityintroduction_requester_
