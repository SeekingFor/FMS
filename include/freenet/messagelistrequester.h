#ifndef _messagelistrequester_
#define _messagelistrequester_

#include "iindexrequester.h"

#include <map>

class MessageListRequester:public IIndexRequester<long>
{
public:
	MessageListRequester();
	MessageListRequester(FCPv2 *fcp);

private:
	void Initialize();
	void PopulateIDList();
	void StartRequest(const long &id);
	void StartRedirectRequest(FCPMessage &message);
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);
	void GetBoardList(std::map<std::string,bool> &boards);

	bool m_localtrustoverrides;
	bool m_savetonewboards;

};

#endif	// _messagelistrequester_
