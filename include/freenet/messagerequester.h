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

	const long GetBoardID(const std::string &boardname, const std::string &identityname);
	const bool SaveToBoard(const std::string &boardname);
	const std::string GetIdentityName(const long identityid);

	int m_maxdaysbackward;
	int m_maxpeermessages;
	int m_maxboardspermessage;
	bool m_savemessagesfromnewboards;
	bool m_localtrustoverrides;
	
};

#endif	// _messagerequester_
