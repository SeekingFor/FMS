#ifndef _identityintroductioninserter_
#define _identityintroductioninserter_

#include "../idatabase.h"
#include "../ilogger.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

#include <Poco/DateTime.h>

class IdentityIntroductionInserter:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IdentityIntroductionInserter(SQLite3DB::DB *db);
	IdentityIntroductionInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp);

	void FCPConnected();
	void FCPDisconnected();

	const bool HandleMessage(FCPv2::Message &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	void Initialize();
	void CheckForNewInserts();
	void StartInsert(const long localidentityid, const std::string &day, const std::string &UUID, const std::string &solution);

	std::string m_messagebase;
	Poco::DateTime m_lastchecked;
	bool m_inserting;
	
};

#endif	// _identityintroductioninserter_
