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
	const bool StartInsert(const long &localidentityid);
	void CheckForNeededInsert();

	long m_daysbackward;
	std::map<long,std::string> m_lastinsertedxml;	// last xml document inserted for each local identity

};

#endif	// _messagelistinserter_
