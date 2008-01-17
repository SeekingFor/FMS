#ifndef _global_
#define _global_

#include <vector>
#include <zthread/Thread.h>

#define FMS_VERSION	"0.0.2"

// opens database and creates tables and initial inserts if necessary
void SetupDB();
// inserts default options into the database
void SetupDefaultOptions();
// opens logfile and sets it up
void SetupLogFile();

void StartThreads(std::vector<ZThread::Thread *> &threads);
void ShutdownThreads(std::vector<ZThread::Thread *> &threads);

// needed for Windows to setup network
void SetupNetwork();
// cleanup network on Windows
void ShutdownNetwork();

#endif	// _global_
