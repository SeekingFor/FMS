#include "../../include/nntp/nntplistener.h"
#include "../../include/nntp/nntpconnection.h"
#include "../../include/option.h"
#include "../../include/global.h"
#include "../../include/stringfunctions.h"

#include <Poco/Net/SocketAddress.h>

#include <cstring>

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

void NNTPListener::run()
{
	int rval;
	fd_set readfs;
	struct timeval tv;
	std::vector<SOCKET>::iterator listeni;
	SOCKET highsocket;

	m_log->debug("NNTPListener::run thread started.");

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
					m_log->information("NNTPListener::run NNTP client connected");
					m_connections.Start(new NNTPConnection(newsock));
				}
			}
		}

	}while(!IsCancelled() && m_listensockets.size()>0);

	m_connections.Cancel();
	m_connections.Join();

	for(listeni=m_listensockets.begin(); listeni!=m_listensockets.end(); listeni++)
	{
		#ifdef _WIN32
		closesocket((*listeni));
		#else
		close((*listeni));
		#endif
	}
	m_listensockets.clear();

	m_log->debug("NNTPListener::run thread exiting.");

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
				Poco::Net::SocketAddress sa(current->ai_addr,current->ai_addrlen);
				m_log->debug("NNTPListener::StartListen trying to create socket, bind, and listen on "+sa.toString());

				sock=socket(current->ai_family,current->ai_socktype,current->ai_protocol);
				if(sock!=INVALID_SOCKET)
				{
					#ifndef _WIN32
					const char optval='1';
					setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
					#endif
					if(bind(sock,current->ai_addr,current->ai_addrlen)==0)
					{
						if(listen(sock,10)==0)
						{
							m_log->information("NNTPListener::StartListen started listening on "+sa.toString());
							m_listensockets.push_back(sock);
						}
						else
						{
							m_log->error("NNTPListener::StartListen socket listen failed");
							#ifdef _WIN32
							closesocket(sock);
							#else
							close(sock);
							#endif
						}
					}
					else
					{
						m_log->error("NNTPListener::StartListen socket bind failed");
						#ifdef _WIN32
						closesocket(sock);
						#else
						close(sock);
						#endif
					}
				}
				else
				{
					m_log->error("NNTPListener::StartListen couldn't create socket");
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
		m_log->fatal("NNTPListener::StartListen couldn't start listening on any sockets");
	}
}
