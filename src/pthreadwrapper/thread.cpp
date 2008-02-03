#include "../../include/pthreadwrapper/thread.h"

#include <sys/timeb.h>

#ifdef XMEM
	#include <xmem.h>
#endif

namespace PThread
{

Thread::Thread()
{
	m_running=false;
	m_cancelled=false;
	m_runnable=0;
}

Thread::Thread(Runnable *runnable)
{
	m_running=false;
	m_cancelled=false;
	m_runnable=runnable;
	if(m_runnable)
	{
		m_runnable->m_thread=this;
		Start();
	}
}

Thread::~Thread()
{
	Cancel();
	Join();
	if(m_runnable)
	{
		delete m_runnable;
	}
}

void Thread::Cancel()
{
	if(m_running)
	{
		m_cancelled=true;
	}
}

void *Thread::EntryPoint(void *pthis)
{
	if(pthis)
	{
		((Thread *)pthis)->m_runnable->Run();
		((Thread *)pthis)->m_running=false;
		((Thread *)pthis)->m_cancelled=false;
	}
	return NULL;
}

void Thread::Join()
{
	if(m_running)
	{
		pthread_join(m_thread,NULL);
	}
}

void Thread::Sleep(const long ms)
{
	if(m_running)
	{
		pthread_cond_t c;
		pthread_mutex_t m;
		timespec t;
		timeb tb;

		pthread_mutex_init(&m,NULL);
		pthread_cond_init(&c,NULL);

		ftime(&tb);

		t.tv_sec=tb.time+(ms/1000);
		t.tv_nsec=((1000000L)*(long)tb.millitm)+((1000000L)*(ms%1000));

		pthread_mutex_lock(&m);
		pthread_cond_timedwait(&c,&m,&t);
		pthread_mutex_unlock(&m);
	}
}

void Thread::Start()
{
	m_running=true;
	pthread_create(&m_thread,NULL,Thread::EntryPoint,this);
}

}	// namespace
