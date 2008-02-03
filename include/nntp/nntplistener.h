#ifndef _nntp_listener_
#define _nntp_listener_

#include <list>
#include <vector>
//#include <zthread/Thread.h>
//#include <zthread/Task.h>
//#include <zthread/ZThread.h>
//#include <zthread/ThreadedExecutor.h>
#include "../pthreadwrapper/runnable.h"

#include "../socketdefines.h"

/**
	\brief Listens for NNTP connections
*/
class NNTPListener:public PThread::Runnable
{
public:
	NNTPListener();
	~NNTPListener();

	void Run();
	void StartListen();

private:

	unsigned short m_listenport;
	std::vector<SOCKET> m_listensockets;
	//ZThread::ThreadedExecutor m_connections;
	std::vector<PThread::Thread *> m_connectionthreads;

};

#endif	// _nntp_listener_
