#ifndef _global_
#define _global_

#include <vector>
//#include <zthread/Thread.h>
#include "pthreadwrapper/thread.h"

#define FMS_VERSION	"0.1.8"

// opens database and creates tables and initial inserts if necessary
void SetupDB();
// inserts default options into the database
void SetupDefaultOptions();
// opens logfile and sets it up
void SetupLogFile();

void StartThreads(std::vector<PThread::Thread *> &threads);
void ShutdownThreads(std::vector<PThread::Thread *> &threads);

// needed for Windows to setup network
void SetupNetwork();
// cleanup network on Windows
void ShutdownNetwork();

#endif	// _global_
