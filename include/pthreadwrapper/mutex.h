#ifndef _pthread_mutex_
#define _pthread_mutex_

#include <pthread.h>
#include "deadlockexception.h"

namespace PThread
{

class Mutex
{
public:
	Mutex();
	~Mutex();

	void Acquire() throw(std::exception);
	void Release();

private:
	pthread_mutex_t m_mutex;
	pthread_mutexattr_t m_attr;
};

}	// namespace

#endif	// _pthread_mutex_
