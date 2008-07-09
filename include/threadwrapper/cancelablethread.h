#ifndef _cancelablethread_
#define _cancelablethread_

#include "cancelablerunnable.h"

#include <Poco/Thread.h>

class CancelableThread:public Poco::Thread
{
public:
	CancelableThread():m_runnable(0)										{}
	CancelableThread(CancelableRunnable *runnable):m_runnable(runnable)	{ start(*runnable); }
	~CancelableThread()
	{
		if(m_runnable)
		{
			delete m_runnable;
		}
	}

	// CancelableThread takes ownership of runnable and will destroy it in the destructor
	void Start(CancelableRunnable *runnable)	{ m_runnable=runnable; start(*runnable); }

	void Cancel()				{ if(m_runnable) { m_runnable->Cancel(); } }
	const bool IsCancelled()	{ return m_runnable ? m_runnable->IsCancelled() : false; }

private:

	CancelableRunnable *m_runnable;

};

#endif	// _cancelablethread_
