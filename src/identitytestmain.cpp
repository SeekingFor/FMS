#include "../include/identitytestglobal.h"
#include "../include/commandthread.h"

#include <ctime>

#ifdef XMEM
	#include <xmem.h>
#endif

int main()
{

	#ifdef XMEM
		xmem_disable_print();
	#endif

	std::vector<ZThread::Thread *> threads;

	srand(time(NULL));

	SetupDB();
	SetupDefaultOptions();


	SetupLogFile();

	SetupNetwork();

	LogFile::instance()->WriteLog(LogFile::LOGLEVEL_INFO,"FMS startup v"FMS_VERSION);

	
	StartThreads(threads);


	ZThread::Thread commandthread(new CommandThread());
	commandthread.wait();


	ShutdownThreads(threads);

	ShutdownNetwork();

	LogFile::instance()->WriteLog(LogFile::LOGLEVEL_INFO,"FMS shutdown");
	LogFile::instance()->WriteNewLine();

	return 0;
}
