#ifndef _commandthread_
#define _commandthread_

#include "ilogger.h"
#include "idatabase.h"

//#include <zthread/Thread.h>
#include "pthreadwrapper/runnable.h"

class CommandThread:public PThread::Runnable,public ILogger, public IDatabase
{
public:

	void Run();

private:

	void HandleInput(const std::string &input);
	
	// methods to handle commands
	void HandleHelpCommand();
	void HandleQuit();
	
	bool m_running;

};

#endif	// _commandthread_
