#ifndef _identityintroduction_requester_
#define _identityintroduction_requester_

#include "../idatabase.h"
#include "../ilogger.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

#include <Poco/DateTime.h>

class IdentityIntroductionRequester:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IdentityIntroductionRequester(SQLite3DB::DB *db);
	IdentityIntroductionRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

	void FCPDisconnected();
	void FCPConnected();
	const bool HandleMessage(FCPv2::Message &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	void Initialize();
	void StartRequests(const long localidentityid);
	void StartRequest(const std::string &UUID);
	void PopulateIDList();
	void RemoveFromRequestList(const std::string &UUID);
	const bool HandleGetFailed(FCPv2::Message &message);
	const bool HandleAllData(FCPv2::Message &message);

	Poco::DateTime m_tempdate;
	std::map<long,bool> m_ids;
	std::vector<std::string> m_requesting;
	std::string m_messagebase;
	int m_maxrequests;

};

#endif	// _identityintroduction_requester_
