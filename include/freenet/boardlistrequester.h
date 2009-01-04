#ifndef _boardlistrequester_
#define _boardlistrequester_

#include "iindexrequester.h"

class BoardListRequester:public IIndexRequester<long>
{
public:
	BoardListRequester();
	BoardListRequester(FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();
	void StartRequest(const long &identityid);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

	std::string GetIdentityName(const long identityid);

	bool m_savemessagesfromnewboards;
	bool m_localtrustoverrides;

};

#endif	// _boardlistrequester_
