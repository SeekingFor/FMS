#ifndef _threadedexecutor_
#define _threadedexecutor_

#include "cancelablerunnable.h"
#include "cancelablethread.h"

#include <vector>

class ThreadedExecutor
{
public:
	~ThreadedExecutor();

	void Start(CancelableRunnable *runnable);

	void Join();
	void Cancel();

private:
	void CleanupCompleted();

	std::vector<CancelableThread *> m_threads;
};

#endif	// _threadedexecutor_
