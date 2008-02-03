#ifndef _pthread_runnable_
#define _pthread_runnable_

#include "thread.h"

namespace PThread
{

class Thread;

class Runnable
{
public:
	Runnable():m_thread(0)	{}
	virtual ~Runnable()		{}

	virtual void Run()=0;

protected:
	void Sleep(const long ms);
	const bool IsCancelled();

private:

	friend class Thread;
	Thread *m_thread;

};

}	// namespace

#endif	// _pthread_runnable_
