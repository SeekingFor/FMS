#include "../../include/threadwrapper/threadedexecutor.h"

ThreadedExecutor::~ThreadedExecutor()
{
	for(std::vector<CancelableThread *>::iterator i=m_threads.begin(); i!=m_threads.end(); i++)
	{
		(*i)->Cancel();
		(*i)->join();
		delete (*i);
	}
}

void ThreadedExecutor::Cancel()
{
	for(std::vector<CancelableThread *>::iterator i=m_threads.begin(); i!=m_threads.end(); i++)
	{
		if((*i)->isRunning())
		{
			(*i)->Cancel();
		}
	}
}

void ThreadedExecutor::CleanupCompleted()
{
	for(std::vector<CancelableThread *>::iterator i=m_threads.begin(); i!=m_threads.end(); )
	{
		if((*i)->isRunning()==false)
		{
			delete (*i);
			i=m_threads.erase(i);
		}
		else
		{
			i++;
		}
	}
}

void ThreadedExecutor::Join()
{
	CleanupCompleted();
	for(std::vector<CancelableThread *>::iterator i=m_threads.begin(); i!=m_threads.end(); i++)
	{
		if((*i)->isRunning())
		{
			(*i)->join();
		}
	}
	CleanupCompleted();
}

void ThreadedExecutor::Start(CancelableRunnable *runnable)
{
	m_threads.push_back(new CancelableThread(runnable));
	CleanupCompleted();
}
