#include "../../include/pthreadwrapper/guard.h"
#include "../../include/pthreadwrapper/mutex.h"

#ifdef XMEM
	#include <xmem.h>
#endif

namespace PThread
{

Guard::Guard()
{
	m_mutex=0;
}

Guard::Guard(Mutex &mutex)
{
	m_mutex=&mutex;
	m_mutex->Acquire();	
}

Guard::~Guard()
{
	if(m_mutex)
	{
		m_mutex->Release();	
	}
}

}	// namespace
