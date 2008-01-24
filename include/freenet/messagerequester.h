#ifndef _messagerequester_
#define _messagerequester_

#include "iindexrequester.h"

class MessageRequester:public IIndexRequester<std::string>
{
public:
	MessageRequester();
	MessageRequester(FCPv2 *fcp);

private:
	void Initialize();
	void PopulateIDList();
	void StartRequest(const std::string &requestid);
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);

	const long GetBoardID(const std::string &boardname);
	const std::string GetIdentityName(const long identityid);

	long m_maxdaysbackward;
	
};

#endif	// _messagerequester_
