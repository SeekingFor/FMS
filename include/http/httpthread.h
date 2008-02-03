#ifndef _httpthread_
#define _httpthread_

#include "../pthreadwrapper/runnable.h"
#include "../ilogger.h"
#include "../idatabase.h"
#include "ipagehandler.h"
#include "httpdefs.h"

#include <cstdlib>
#include <shttpd.h>

class HTTPThread:public PThread::Runnable,public ILogger, public IDatabase
{
public:
	HTTPThread();
	~HTTPThread();
	
	void Run();

private:
	static void PageCallback(shttpd_arg *arg);

	struct shttpd_ctx *m_ctx;

	std::vector<IPageHandler *> m_pagehandlers;

};

#endif	// _httpthread_
