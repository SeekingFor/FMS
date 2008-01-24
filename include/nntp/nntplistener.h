#ifndef _nntp_listener_
#define _nntp_listener_

#include <list>
#include <vector>
#include <zthread/Task.h>
#include <zthread/ZThread.h>
#include <zthread/ThreadedExecutor.h>

#include "../socketdefines.h"

/**
	\brief Listens for NNTP connections
*/
class NNTPListener:public ZThread::Runnable
{
public:
	NNTPListener();
	~NNTPListener();

	void run();
	void StartListen();

private:

	unsigned short m_listenport;
	std::vector<SOCKET> m_listensockets;
	ZThread::ThreadedExecutor m_connections;

};

#endif	// _nntp_listener_
