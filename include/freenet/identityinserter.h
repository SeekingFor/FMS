#ifndef _identity_inserter_
#define _identity_inserter_

#include "../idatabase.h"
#include "../ilogger.h"
#include "../datetime.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

class IdentityInserter:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IdentityInserter();
	IdentityInserter(FCPv2 *fcp);

	void FCPConnected();
	void FCPDisconnected();

	const bool HandleMessage(FCPMessage &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	void Initialize();
	void CheckForNeededInsert();
	void StartInsert(const long localidentityid);

	DateTime m_lastchecked;

};

#endif	// _identity_inserter_
