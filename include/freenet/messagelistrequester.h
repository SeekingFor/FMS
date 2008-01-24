#ifndef _messagelistrequester_
#define _messagelistrequester_

#include "iindexrequester.h"

class MessageListRequester:public IIndexRequester<long>
{
public:
	MessageListRequester();
	MessageListRequester(FCPv2 *fcp);

private:
	void Initialize();
	void PopulateIDList();
	void StartRequest(const long &id);
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);

};

#endif	// _messagelistrequester_
