#ifndef _nntp_listener_
#define _nntp_listener_

#include <list>
#include <vector>
#include "../threadwrapper/threadedexecutor.h"
#include "../socketdefines.h"
#include "../ilogger.h"
#include "../ithreaddatabase.h"

/**
	\brief Listens for NNTP connections
*/
class NNTPListener:public CancelableRunnable,public ILogger,public IThreadDatabase
{
public:
	NNTPListener();
	~NNTPListener();

	void run();

private:

	void StartListen();

	unsigned short m_listenport;
	std::vector<SOCKET> m_listensockets;

	ThreadedExecutor m_connections;

};

#endif	// _nntp_listener_
