#ifndef _messagelistinserter_
#define _messagelistinserter_

#include "iindexinserter.h"

class MessageListInserter:public IIndexInserter<long>
{
public:
	MessageListInserter();
	MessageListInserter(FCPv2 *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPMessage &message);
	const bool HandlePutFailed(FCPMessage &message);
	void StartInsert(const long &localidentityid);
	void CheckForNeededInsert();

	long m_daysbackward;

};

#endif	// _messagelistinserter_
