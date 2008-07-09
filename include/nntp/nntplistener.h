#ifndef _nntp_listener_
#define _nntp_listener_

#include <list>
#include <vector>
#include "../threadwrapper/threadedexecutor.h"
#include "../socketdefines.h"
#include "../ilogger.h"

/**
	\brief Listens for NNTP connections
*/
class NNTPListener:public CancelableRunnable,public ILogger
{
public:
	NNTPListener();
	~NNTPListener();

	void run();
	void StartListen();

private:

	unsigned short m_listenport;
	std::vector<SOCKET> m_listensockets;

	ThreadedExecutor m_connections;

};

#endif	// _nntp_listener_
