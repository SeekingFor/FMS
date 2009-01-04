#ifndef _introductionpuzzlerequester_
#define _introductionpuzzlerequester_

#include "iindexrequester.h"

#include <Poco/DateTime.h>

class IntroductionPuzzleRequester:public IIndexRequester<long>
{
public:
	IntroductionPuzzleRequester();
	IntroductionPuzzleRequester(FCPv2::Connection *fcp);

private:
	void Initialize();
	void StartRequest(const long &identityid);
	void PopulateIDList();				// clear and re-populate m_ids with identities we want to query
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

};

#endif	// _introductionpuzzlerequester_
