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

const int ThreadedExecutor::Start(CancelableRunnable *runnable)
{
	CleanupCompleted();
	CancelableThread *newthread=new CancelableThread(runnable);
	if(newthread)
	{
		m_threads.push_back(newthread);
		return newthread->GetThreadID();
	}
	else
	{
		return 0;
	}
}

const int ThreadedExecutor::Start(CancelableRunnable *runnable, const std::string &name)
{
	CleanupCompleted();
	CancelableThread *newthread=new CancelableThread(runnable,name);
	if(newthread)
	{
		m_threads.push_back(newthread);
		return newthread->GetThreadID();
	}
	else
	{
		return 0;
	}
}
