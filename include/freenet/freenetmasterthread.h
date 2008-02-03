#ifndef _freenetmasterthread_
#define _freenetmasterthread_

#include "../ilogger.h"
#include "ifreenetregistrable.h"
#include "ifcpmessagehandler.h"
#include "ifcpconnected.h"
#include "iperiodicprocessor.h"

//#include <zthread/Runnable.h>
#include "../pthreadwrapper/runnable.h"

// forward declaration
class IFreenetRegistrable;

class FreenetMasterThread:public PThread::Runnable,public ILogger, public IFCPMessageHandler
{
public:
	FreenetMasterThread();
	~FreenetMasterThread();
	
	const bool HandleMessage(FCPMessage &message);

	void Run();

	// registration methods for children objects
	void RegisterPeriodicProcessor(IPeriodicProcessor *obj);
	void RegisterFCPConnected(IFCPConnected *obj);
	void RegisterFCPMessageHandler(IFCPMessageHandler *obj);

private:
	const bool FCPConnect();
	void Setup();
	void Shutdown();

	std::string m_fcphost;
	long m_fcpport;
	FCPv2 m_fcp;
	std::vector<IFreenetRegistrable *> m_registrables;
	std::vector<IPeriodicProcessor *> m_processors;
	std::vector<IFCPConnected *> m_fcpconnected;
	std::vector<IFCPMessageHandler *> m_fcpmessagehandlers;
	bool m_receivednodehello;

};

#endif	// _freenetmasterthread_
