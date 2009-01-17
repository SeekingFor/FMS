#ifndef _dbmaintenancethread_
#define _dbmaintenancethread_

#include "threadwrapper/cancelablerunnable.h"
#include "ilogger.h"
#include "ithreaddatabase.h"

#include <Poco/DateTime.h>

class DBMaintenanceThread:public CancelableRunnable,public ILogger,public IThreadDatabase
{
public:
	DBMaintenanceThread();

	void run();

private:

	void Do10MinuteMaintenance();
	void Do30MinuteMaintenance();
	void Do1HourMaintenance();
	void Do6HourMaintenance();
	void Do1DayMaintenance();

	Poco::DateTime m_last10minute;
	Poco::DateTime m_last30minute;
	Poco::DateTime m_last1hour;
	Poco::DateTime m_last6hour;
	Poco::DateTime m_last1day;

	long m_deletemessagesolderthan;
	long m_messagedownloadmaxdaysbackward;

};

#endif	// _dbmaintenancethread_
