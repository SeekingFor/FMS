#ifndef _freenetmasterthread_
#define _freenetmasterthread_

#include "../ilogger.h"
#include "../ithreaddatabase.h"
#include "ifreenetregistrable.h"
#include "ifcpmessagehandler.h"
#include "ifcpconnected.h"
#include "iperiodicprocessor.h"

#include "../threadwrapper/cancelablerunnable.h"

// forward declaration
class IFreenetRegistrable;

class FreenetMasterThread:public CancelableRunnable,public ILogger, public IFCPMessageHandler, public IThreadDatabase
{
public:
	FreenetMasterThread();
	~FreenetMasterThread();
	
	const bool HandleMessage(FCPv2::Message &message);

	void run();

	// registration methods for child objects
	void RegisterPeriodicProcessor(IPeriodicProcessor *obj);
	void RegisterFCPConnected(IFCPConnected *obj);
	void RegisterFCPMessageHandler(IFCPMessageHandler *obj);

private:
	const bool FCPConnect();
	void Setup();
	void Shutdown();

	std::string m_fcphost;
	int m_fcpport;
	FCPv2::Connection m_fcp;
	std::vector<IFreenetRegistrable *> m_registrables;
	std::vector<IPeriodicProcessor *> m_processors;
	std::vector<IFCPConnected *> m_fcpconnected;
	std::vector<IFCPMessageHandler *> m_fcpmessagehandlers;
	bool m_receivednodehello;

};

#endif	// _freenetmasterthread_
