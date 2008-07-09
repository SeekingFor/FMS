#ifndef _fmsversionrequester_
#define _fmsversionrequester_

#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../ilogger.h"

#include <Poco/DateTime.h>

class FMSVersionRequester:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	FMSVersionRequester();
	FMSVersionRequester(FCPv2 *fcp);

	void FCPConnected()			{}
	void FCPDisconnected()		{}
	const bool HandleMessage(FCPMessage &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);
	void Initialize();
	void StartRequest();

	Poco::DateTime m_lastchecked;
	std::string m_fcpuniquename;
};

#endif	// _fmsversionrequester_
