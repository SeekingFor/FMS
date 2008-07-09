#ifndef _httpthread_
#define _httpthread_

#include "../threadwrapper/cancelablerunnable.h"
#include "../ilogger.h"
#include "../idatabase.h"

#include <cstdlib>

class HTTPThread:public CancelableRunnable,public ILogger, public IDatabase
{
public:
	HTTPThread();
	
	void run();

private:
	//static void PageCallback(shttpd_arg *arg);

	int m_listenport;

};

#endif	// _httpthread_
