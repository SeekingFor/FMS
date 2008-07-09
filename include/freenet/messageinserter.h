#ifndef _messageinserter_
#define _messageinserter_

#include "iindexinserter.h"

// only handle 1 insert at a time
class MessageInserter:public IIndexInserter<std::string>
{
public:
	MessageInserter();
	MessageInserter(FCPv2 *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPMessage &message);
	const bool HandlePutFailed(FCPMessage &message);
	const bool StartInsert(const std::string &messageuuid);
	void CheckForNeededInsert();

};

#endif	// _messageinserter_
