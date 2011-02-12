#ifndef _identity_redirect_inserter_
#define _identity_redirect_inserter_

#include "../idatabase.h"
#include "../ilogger.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

#include <Poco/DateTime.h>

class IdentityRedirectInserter:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IdentityRedirectInserter(SQLite3DB::DB *db);
	IdentityRedirectInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp);

	void FCPConnected();
	void FCPDisconnected();

	const bool HandleMessage(FCPv2::Message &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	void Initialize();
	void CheckForNeededInsert();
	void StartInsert(const long localidentityid, const std::string &redirect);

	Poco::DateTime m_lastchecked;
};

#endif	// _identity_redirect_inserter_
