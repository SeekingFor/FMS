#ifndef _introductionpuzzlerequester_
#define _introductionpuzzlerequester_

#include "iindexrequester.h"

#include <Poco/DateTime.h>

class IntroductionPuzzleRequester:public IIndexRequester<long>
{
public:
	IntroductionPuzzleRequester();
	IntroductionPuzzleRequester(FCPv2 *fcp);

private:
	void Initialize();
	void StartRequest(const long &identityid);
	void PopulateIDList();				// clear and re-populate m_ids with identities we want to query
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);

};

#endif	// _introductionpuzzlerequester_
