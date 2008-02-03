/**
	Light C++ wrapper around PThreads
*/

#ifndef _pthread_wrapper_

#include <pthread.h>

namespace PThread
{

	// forward declarations
	class NonCopyable;
	class Mutex;
	class Guard;
	class Runnable;
	class Thread;

}	// namespace

#include "pthreadwrapper/noncopyable.h"
#include "pthreadwrapper/mutex.h"
#include "pthreadwrapper/guard.h"
#include "pthreadwrapper/runnable.h"
#include "pthreadwrapper/thread.h"
#include "pthreadwrapper/singleton.h"

#endif	// _pthread_wrapper_
