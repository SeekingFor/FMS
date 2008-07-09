#ifndef _trustlistinserter_
#define _trustlistinserter_

#include "../idatabase.h"
#include "../ilogger.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

#include <Poco/DateTime.h>

class TrustListInserter:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	TrustListInserter();
	TrustListInserter(FCPv2 *fcp);

	void FCPConnected();
	void FCPDisconnected();

	const bool HandleMessage(FCPMessage &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	void Initialize();
	void CheckForNeededInsert();
	void StartInsert(const long localidentityid, const std::string &privatekey);

	std::string m_messagebase;
	Poco::DateTime m_lastchecked;
	
};

#endif	// _trustlistinserter_
