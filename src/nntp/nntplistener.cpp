#include "../../include/nntp/nntplistener.h"
#include "../../include/nntp/nntpconnection.h"
#include "../../include/option.h"
#include "../../include/logfile.h"
#include "../../include/global.h"
#include "../../include/stringfunctions.h"

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <netinet/in.h>  // gcc - IPPROTO_ consts
	#include <netdb.h>       // gcc - addrinfo
#endif

#ifdef XMEM
	#include <xmem.h>
#endif

NNTPListener::NNTPListener()
{

}

NNTPListener::~NNTPListener()
{

}

void NNTPListener::Run()
{
	int rval;
	fd_set readfs;
	struct timeval tv;
	std::vector<SOCKET>::iterator listeni;
	SOCKET highsocket;

	LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_DEBUG,"NNTPListener::run thread started.");

	StartListen();

	do
	{
		// reset values
		highsocket=0;
		tv.tv_sec=1;
		tv.tv_usec=0;

		// clear fd set
		FD_ZERO(&readfs);

		// put all listen sockets on the fd set
		for(listeni=m_listensockets.begin(); listeni!=m_listensockets.end(); listeni++)
		{
			FD_SET((*listeni),&readfs);
			if((*listeni)>highsocket)
			{
				highsocket=(*listeni);
			}
		}

		// see if any connections are waiting
		rval=select(highsocket+1,&readfs,0,0,&tv);

		// check for new connections
		if(rval>0)
		{
			for(listeni=m_listensockets.begin(); listeni!=m_listensockets.end(); listeni++)
			{
				if(FD_ISSET((*listeni),&readfs))
				{
					SOCKET newsock;
					struct sockaddr_storage addr;
					socklen_t addrlen=sizeof(addr);
					newsock=accept((*listeni),(struct sockaddr *)&addr,&addrlen);
					LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_INFO,"NNTPListener::run NNTP client connected");
					//m_connections.execute(new NNTPConnection(newsock));
					m_connectionthreads.push_back(new PThread::Thread(new NNTPConnection(newsock)));
				}
			}
		}

		// check for any non-running connection threads that we can delete
		for(std::vector<PThread::Thread *>::iterator i=m_connectionthreads.begin(); i!=m_connectionthreads.end(); )
		{
			if((*i)->IsRunning()==false)
			{
				delete (*i);
				i=m_connectionthreads.erase(i);
			}
			if(i!=m_connectionthreads.end())
			{
				i++;
			}
		}

	//}while(!ZThread::Thread::interrupted() && m_listensockets.size()>0);
	}while(!IsCancelled() && m_listensockets.size()>0);

	// see if any threads are still running - just calling interrupt without check would cause assert in debug mode
	/*
	if(m_connections.wait(1)==false)
	{
		LogFile::instance()->WriteLog(LogFile::LOGLEVEL_DEBUG,"NNTPListener::run interrupting connection threads and waiting 60 seconds for exit.");
		try
		{
			m_connections.interrupt();
		}
		catch(...)
		{
			LogFile::instance()->WriteLog(LogFile::LOGLEVEL_DEBUG,"NNTPListener::run caught unhandled exception.");
		}
		if(m_connections.wait(60000)==false)
		{
			LogFile::instance()->WriteLog(LogFile::LOGLEVEL_DEBUG,"NNTPListener::run connection threads did not exit after 60 seconds.");
		}
	}
	*/
	for(std::vector<PThread::Thread *>::iterator i=m_connectionthreads.begin(); i!=m_connectionthreads.end(); i++)
	{
		if((*i)->IsRunning())
		{
			LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_DEBUG,"NNTPListener::Run waiting for connection thread to exit.");
			(*i)->Cancel();
			(*i)->Join();
		}
		delete (*i);
	}

	for(listeni=m_listensockets.begin(); listeni!=m_listensockets.end(); listeni++)
	{
		#ifdef _WIN32
		closesocket((*listeni));
		#else
		close((*listeni));
		#endif
	}
	m_listensockets.clear();

	LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_DEBUG,"NNTPListener::run thread exiting.");

}

void NNTPListener::StartListen()
{
	
	std::string bindaddresses;
	std::vector<std::string> listenaddresses;
	std::string nntpport;
	if(Option::Instance()->Get("NNTPListenPort",nntpport)==false)
	{
		nntpport="1119";
		Option::Instance()->Set("NNTPListenPort",nntpport);
	}
	if(Option::Instance()->Get("NNTPBindAddresses",bindaddresses)==false)
	{
		bindaddresses="127.0.0.1";
		Option::Instance()->Set("NNTPBindAddresses",bindaddresses);
	}
	StringFunctions::Split(bindaddresses,",",listenaddresses);
	
	for(std::vector<std::string>::iterator i=listenaddresses.begin(); i!=listenaddresses.end(); i++)
	{
		SOCKET sock;
		int rval;
		struct addrinfo hint,*result,*current;
		result=current=NULL;
		memset(&hint,0,sizeof(hint));
		hint.ai_socktype=SOCK_STREAM;
		hint.ai_protocol=IPPROTO_TCP;
		hint.ai_flags=AI_PASSIVE;
		
		rval=getaddrinfo((*i).c_str(),nntpport.c_str(),&hint,&result);
		if(rval==0)
		{
			for(current=result; current!=NULL; current=current->ai_next)
			{
				sock=socket(current->ai_family,current->ai_socktype,current->ai_protocol);
				if(sock!=INVALID_SOCKET)
				{
					if(bind(sock,current->ai_addr,current->ai_addrlen)==0)
					{
						if(listen(sock,10)==0)
						{
							LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_INFO,"NNTPListener::StartListen started listening at "+(*i)+":"+nntpport);
							m_listensockets.push_back(sock);
						}
						else
						{
							LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_ERROR,"NNTPListener::StartListen socket listen failed");
							#ifdef _WIN32
							closesocket(sock);
							#else
							close(sock);
							#endif
						}
					}
					else
					{
						LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_ERROR,"NNTPListener::StartListen socket bind failed");
						#ifdef _WIN32
						closesocket(sock);
						#else
						close(sock);
						#endif
					}
				}
				else
				{
					LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_ERROR,"NNTPListener::StartListen couldn't create socket");
				}
			}
		}
		if(result)
		{
			freeaddrinfo(result);
		}
	}
	if(m_listensockets.size()==0)
	{
		LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_FATAL,"NNTPListener::StartListen couldn't start listening on any sockets");
	}
}
