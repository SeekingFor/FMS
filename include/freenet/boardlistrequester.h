#ifndef _boardlistrequester_
#define _boardlistrequester_

#include "iindexrequester.h"

class BoardListRequester:public IIndexRequester<long>
{
public:
	BoardListRequester();
	BoardListRequester(FCPv2 *fcp);

private:
	void Initialize();
	void PopulateIDList();
	void StartRequest(const long &identityid);
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);

	std::string GetIdentityName(const long identityid);

	bool m_savemessagesfromnewboards;
	bool m_localtrustoverrides;

};

#endif	// _boardlistrequester_
