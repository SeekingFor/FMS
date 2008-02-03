#include "../../include/pthreadwrapper/runnable.h"

#ifdef XMEM
	#include <xmem.h>
#endif

namespace PThread
{

const bool Runnable::IsCancelled()
{
	if(m_thread)
	{
		return m_thread->IsCancelled();
	}
	else
	{
		return false;
	}
}

void Runnable::Sleep(const long ms)
{
	if(m_thread)
	{
		m_thread->Sleep(ms);
	}
}

}	// namespace
