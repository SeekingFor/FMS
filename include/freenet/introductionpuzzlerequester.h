#ifndef _introductionpuzzlerequester_
#define _introductionpuzzlerequester_

#include "../idatabase.h"
#include "../ilogger.h"
#include "../datetime.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

class IntroductionPuzzleRequester:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IntroductionPuzzleRequester();
	IntroductionPuzzleRequester(FCPv2 *fcp);

	void FCPDisconnected();
	void FCPConnected();

	const bool HandleMessage(FCPMessage &message);

	void RegisterWithThread(FreenetMasterThread *thread);

	void Process();

private:
	void Initialize();
	void RemoveFromRequestList(const long identityid);
	void StartRequest(const long identityid);
	void PopulateIDList();				// clear and re-populate m_ids with identities we want to query
	const bool HandleAllData(FCPMessage &message);
	const bool HandleGetFailed(FCPMessage &message);

	DateTime m_tempdate;
	std::string m_messagebase;
	long m_maxrequests;
	std::vector<long> m_requesting;		// list of ids we are currently requesting from
	std::map<long,bool> m_ids;			// map of all ids we know and whether we have requested file from them yet
	
};

#endif	// _introductionpuzzlerequester_
