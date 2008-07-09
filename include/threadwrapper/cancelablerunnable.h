#ifndef _cancelablerunnable_
#define _cancelablerunnable_

#include <Poco/Runnable.h>
#include <Poco/Mutex.h>

class CancelableRunnable:public Poco::Runnable
{
public:
	CancelableRunnable():m_cancelled(false)	{}

	void Cancel()				{ Poco::ScopedLock<Poco::FastMutex> g(m_cancelledmutex); m_cancelled=true; }
	const bool IsCancelled()	{ Poco::ScopedLock<Poco::FastMutex> g(m_cancelledmutex); return m_cancelled; }

private:

	Poco::FastMutex m_cancelledmutex;
	bool m_cancelled;

};

#endif	// _cancelablerunnable_
