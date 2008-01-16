#ifndef _introductionpuzzle_inserter_
#define _introductionpuzzle_inserter_

#include "../idatabase.h"
#include "../ilogger.h"
#include "../datetime.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

class IntroductionPuzzleInserter:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IntroductionPuzzleInserter();
	IntroductionPuzzleInserter(FCPv2 *fcp);

	void FCPConnected();
	void FCPDisconnected();

	const bool HandleMessage(FCPMessage &message);

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:
	void Initialize();
	void CheckForNeededInsert();
	void StartInsert(const long localidentityid);
	void GenerateCaptcha(std::string &encodeddata, std::string &solution);
	const bool HandlePutSuccessful(FCPMessage &message);
	const bool HandlePutFailed(FCPMessage &message);

	DateTime m_lastchecked;

};

#endif	// _introductionpuzzle_inserter_
