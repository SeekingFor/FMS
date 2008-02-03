#include "../include/global.h"
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

	std::vector<PThread::Thread *> threads;

	srand(time(NULL));

	SetupDB();
	SetupDefaultOptions();


	SetupLogFile();

	SetupNetwork();

	LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_INFO,"FMS startup v"FMS_VERSION);


	StartThreads(threads);


	//ZThread::Thread commandthread(new CommandThread());
	PThread::Thread commandthread(new CommandThread());
	commandthread.Join();


	ShutdownThreads(threads);

	ShutdownNetwork();

	LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_INFO,"FMS shutdown");
	LogFile::Instance()->WriteNewLine();

	return 0;
}
