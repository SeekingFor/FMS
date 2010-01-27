#ifndef _identity_inserter_
#define _identity_inserter_

#include "../idatabase.h"
#include "../ilogger.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

#include <Poco/DateTime.h>

class IdentityInserter:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IdentityInserter(SQLite3DB::DB *db);
	IdentityInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp);

	void FCPConnected();
	void FCPDisconnected();

	const bool HandleMessage(FCPv2::Message &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	void Initialize();
	void CheckForNeededInsert();
	void StartInsert(const long localidentityid, const int dayoffset=0);

	Poco::DateTime m_lastchecked;
	Poco::DateTime m_lastreceivedmessage;

};

#endif	// _identity_inserter_
