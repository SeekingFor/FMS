#ifndef _pthread_thread_
#define _pthread_thread_

#include "noncopyable.h"
#include "runnable.h"
#include <pthread.h>

namespace PThread
{

class Runnable;

class Thread:public NonCopyable
{
public:
	Thread();
	Thread(Runnable *runnable);
	~Thread();

	void Join();
	void Cancel();

	void Sleep(const long ms);
	const bool IsCancelled()	{ return m_cancelled; }
	const bool IsRunning()		{ return m_running; }

private:
	void Start();
	static void *EntryPoint(void *pthis);

	pthread_t m_thread;
	bool m_running;				// thread (object) is currently running
	bool m_cancelled;
	Runnable *m_runnable;		// actual object that is being run

};

}	// namespace

#endif	// _pthread_thread_
