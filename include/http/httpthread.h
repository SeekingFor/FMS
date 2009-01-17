#ifndef _httpthread_
#define _httpthread_

#include "../threadwrapper/cancelablerunnable.h"
#include "../ilogger.h"
#include "../ithreaddatabase.h"

#include <cstdlib>

class HTTPThread:public CancelableRunnable,public ILogger, public IThreadDatabase
{
public:
	HTTPThread();
	
	void run();

private:
	//static void PageCallback(shttpd_arg *arg);

	int m_listenport;

};

#endif	// _httpthread_
