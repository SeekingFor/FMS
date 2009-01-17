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
	FMSVersionRequester(SQLite3DB::DB *db);
	FMSVersionRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

	void FCPConnected()			{}
	void FCPDisconnected()		{}
	const bool HandleMessage(FCPv2::Message &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);
	void Initialize();
	void StartRequest();

	Poco::DateTime m_lastchecked;
	std::string m_fcpuniquename;
};

#endif	// _fmsversionrequester_
