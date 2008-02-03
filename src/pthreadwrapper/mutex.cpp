#include "../../include/pthreadwrapper/mutex.h"
#include <exception>

#ifndef _WIN32
	#include <sys/errno.h>
#endif

#ifdef XMEM
	#include <xmem.h>
#endif

namespace PThread
{

Mutex::Mutex()
{
	pthread_mutexattr_init(&m_attr);
	pthread_mutexattr_settype(&m_attr,PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutex_init(&m_mutex,&m_attr);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&m_mutex);
	pthread_mutexattr_destroy(&m_attr);
}

void Mutex::Acquire() throw(std::exception)
{
	int rval=0;
	if((rval=pthread_mutex_lock(&m_mutex))!=0)
	{
		// deadlock - throw exception
		if(rval==EDEADLK)
		{
			throw DeadlockException();
		}
	}
}

void Mutex::Release()
{
	pthread_mutex_unlock(&m_mutex);
}

}	// namespace
