#ifndef _cancelablethread_
#define _cancelablethread_

#include "cancelablerunnable.h"

#include <Poco/Thread.h>

/*
	Poco::Thread does not have a virtual destructor, so we can't inherit safely.
*/
class CancelableThread
{
public:
	CancelableThread():m_runnable(0),m_thread(new Poco::Thread),m_wasjoined(false)								{}
	CancelableThread(const std::string &name):m_runnable(0),m_thread(new Poco::Thread(name)),m_wasjoined(false)	{}
	CancelableThread(CancelableRunnable *runnable):m_runnable(runnable),m_thread(new Poco::Thread),m_wasjoined(false)									{ if(m_thread && m_runnable) { m_thread->start(*runnable); } }
	CancelableThread(CancelableRunnable *runnable, const std::string &name):m_runnable(runnable),m_thread(new Poco::Thread(name)),m_wasjoined(false)	{ if(m_thread && m_runnable) { m_thread->start(*runnable); } }
	~CancelableThread()
	{
		if(m_thread)
		{
			if(m_thread->isRunning() && IsCancelled()==false)
			{
				Cancel();
			}
			if(m_wasjoined==false)
			{
				try
				{
					m_thread->tryJoin(5000);
				}
				catch(...)
				{
				}
			}
			delete m_thread;
		}
		if(m_runnable)
		{
			delete m_runnable;
		}
	}

	// CancelableThread takes ownership of runnable and will destroy it in the destructor
	void Start(CancelableRunnable *runnable)	{ m_runnable=runnable; m_thread->start(*runnable); }

	void Cancel()				{ if(m_runnable) { m_runnable->Cancel(); } }
	const bool IsCancelled()	{ return m_runnable ? m_runnable->IsCancelled() : false; }

	// these methods implemented from Poco::Thread
	void join()					{ if(m_thread) { m_thread->join(); m_wasjoined=true; } }
	bool isRunning() const		{ if(m_thread) { return m_thread->isRunning(); } else { return false; } }

	const int GetThreadID() const
	{
		if(m_thread)
		{
#if defined(POCO_VERSION) && POCO_VERSION>=0x01030600
			return static_cast<int>(m_thread->tid());
#else
			return static_cast<int>(m_thread->id());
#endif
		}
		return 0;
	}

private:

	Poco::Thread *m_thread;
	CancelableRunnable *m_runnable;
	bool m_wasjoined;				// We must call join on ALL Poco threads, running or not, before deletion.  Otherwise there is a potential TLS leak in POSIX Thread implementation.

};

#endif	// _cancelablethread_
