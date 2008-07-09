#include "../../include/freenet/fcpv2.h"
#include <cstdio>
#include <cstdarg>
#include <sstream>
#include <cstring>

#ifdef _WIN32
	#include <ws2tcpip.h>
#else
	#include <netdb.h>
	#include <netinet/in.h>
#endif

/* XMEM doesn't play nice with strtok - should replace strtok with something else anyway
#ifdef XMEM
	#include <xmem.h>
#endif
*/

#ifdef _WIN32
	bool FCPv2::m_wsastartup=false;
#endif



FCPv2::FCPv2()
{
#ifdef _WIN32
	if(m_wsastartup==false)
	{
		WSAData wsadata;
		WSAStartup(MAKEWORD(2,2),&wsadata);
		m_wsastartup=true;
	}
#endif

	// initialize socket to server
	m_serversocket=-1;

	// initialize buffers
	m_tempbuffer=new char[65535];

}


FCPv2::~FCPv2()
{
	Disconnect();
#ifdef _WIN32
	WSACleanup();
#endif

	delete [] m_tempbuffer;

}


const bool FCPv2::Connect(const char *host, const int port)
{
	// disconnect socket to server if it is currently open
	if(Connected())
	{
		Disconnect();
	}

	int rval=-1;
	struct sockaddr_storage m_serveraddr;

	std::ostringstream portstring;
	addrinfo hint,*result,*current;
	result=NULL;
	portstring << port;

	memset(&hint,0,sizeof(addrinfo));
	hint.ai_socktype=SOCK_STREAM;
	rval=getaddrinfo(host,portstring.str().c_str(),&hint,&result);

	// erase any data in buffers
	m_sendbuffer.clear();
	m_receivebuffer.clear();

	if(result)
	{
		for(current=result; current!=NULL && m_serversocket==-1; current=current->ai_next)
		{
			memset(&m_serveraddr,0,sizeof(struct sockaddr_storage));

			m_serversocket=socket(current->ai_family,current->ai_socktype,current->ai_protocol);

			if(m_serversocket!=-1)
			{
				rval=connect(m_serversocket,current->ai_addr,current->ai_addrlen);
				if(rval==-1)
				{
					Disconnect();
				}
			}
		}

		freeaddrinfo(result);
	}

	if(rval==0)
	{
		return true;
	}
	else
	{
		return false;
	}

}

const bool FCPv2::Disconnect()
{
	if(Connected())
	{
	#ifdef _WIN32
		closesocket(m_serversocket);
	#else
		close(m_serversocket);
	#endif
		m_serversocket=-1;
	}
	return true;
}

int FCPv2::FindOnReceiveBuffer(const char *text)
{
	bool found;
	std::vector<char>::size_type i,j;
	size_t tlen=strlen(text);

	if(m_receivebuffer.size()>=tlen)
	{
		for(i=0; i<=m_receivebuffer.size()-tlen; i++)
		{
			found=true;
			for(j=0; j<tlen; j++)
			{
				if(m_receivebuffer[i+j]!=text[j])
				{
					found=false;
					j=tlen;
				}
			}
			if(found==true)
			{
				return i;
			}
		}
	}

	return -1;
}

FCPMessage FCPv2::ReceiveMessage()
{
	int field=0;
	int len=0;
	int endlen=0;
	int endmessage=-1;
	char *buffpos;
	char *prevpos;
	char *buffer;

	FCPMessage message;

	// there is data on the receive buffer
	if(m_receivebuffer.size()>0)
	{

		// find Data on a line by itself following AllData
		if(FindOnReceiveBuffer("AllData\n")==0)
		{
			endmessage=FindOnReceiveBuffer("\nData\n");
			if(endmessage!=-1)
			{
				endmessage++;
				endlen=5;
			}
		}
		// otherwise this is a regular message - search for EndMessage
		else
		{
			endmessage=FindOnReceiveBuffer("EndMessage\n");
			endlen=11;
		}

		// continue if we found "EndMessage\n" or "Data\n"
		if(endmessage!=-1)
		{
			// total length of message (including ending \n)
			len=endmessage+endlen;

			// allocate space for message
			buffer=new char[len+1];

			// copy message from receive buffer to message buffer
			std::copy(m_receivebuffer.begin(),m_receivebuffer.begin()+len,buffer);
			buffer[len]='\0';

			// remove from receive buffer
			m_receivebuffer.erase(m_receivebuffer.begin(),m_receivebuffer.begin()+len);

			// set buffer position
			buffpos=buffer;

			// find message name
			buffpos=strtok(buffer,"\n");
			message.SetName(buffer);

			do
			{
				// find next field
				prevpos=buffpos;
				buffpos=strtok(NULL,"=");

				// continue if we aren't at the end of a regular message, or at Data for an AllData message
				if(strncmp(buffpos,"EndMessage\n",11)!=0 && strncmp(buffpos,"Data\n",5)!=0)	//!(strncmp(message->MessageName,"AllData",7)==0 && strncmp(buffpos,"Data\n",5)==0))
				{

					// find next value
					prevpos=buffpos;
					buffpos=strtok(NULL,"\n");

					if(prevpos && buffpos)
					{
						message[prevpos]=buffpos;
					}

					field++;
				}
				else
				{
					buffpos=0;
				}

			}while(buffpos!=0);

			delete [] buffer;

		}
	}

	return message;
}

const long FCPv2::ReceiveRaw(char *data, long &datalen)
{
	long len=0;
	if(m_receivebuffer.size()>0 && datalen>0)
	{
		if(datalen>m_receivebuffer.size())
		{
			len=m_receivebuffer.size();
		}
		else
		{
			len=datalen;
		}

		std::copy(m_receivebuffer.begin(),m_receivebuffer.begin()+len,data);

		// erase bytes from receive buffer
		m_receivebuffer.erase(m_receivebuffer.begin(),m_receivebuffer.begin()+len);

	}
	datalen=len;
	return datalen;
}

void FCPv2::SendBufferedText(const char *text)
{
	unsigned int i;
	for(i=0; i<strlen(text); i++)
	{
		m_sendbuffer.push_back(text[i]);
	}
}

void FCPv2::SendBufferedRaw(const char *data, const long len)
{
	int i;
	for(i=0; i<len; i++)
	{
		m_sendbuffer.push_back(data[i]);
	}
}

const int FCPv2::SendMessage(const char *messagename, const int fieldcount, ...)
{
	va_list args;
	const char *field;
	const char *val;
	std::vector<char>::size_type bytecount=0;
	int i;
	std::vector<char>::size_type startlen;

	startlen=m_sendbuffer.size();

	SendBufferedText(messagename);
	SendBufferedText("\n");

	va_start(args,fieldcount);

	for(i=0; i<fieldcount; i++)
	{
		field=va_arg(args,const char *);
		val=va_arg(args,const char *);

		SendBufferedText(field);
		SendBufferedText("=");
		SendBufferedText(val);
		SendBufferedText("\n");
	}

	SendBufferedText("EndMessage\n");

	bytecount=m_sendbuffer.size()-startlen;
	
	va_end(args);

	return bytecount;
}

const int FCPv2::SendMessage(FCPMessage &message)
{
	std::vector<char>::size_type bytecount=0;
	std::vector<char>::size_type startlen;
	FCPMessage::iterator i;

	startlen=m_sendbuffer.size();

	if(message.GetName()!="")
	{
		SendBufferedText(message.GetName().c_str());
		SendBufferedText("\n");

		for(i=message.begin(); i!=message.end(); i++)
		{
			SendBufferedText((*i).first.c_str());
			SendBufferedText("=");
			SendBufferedText((*i).second.c_str());
			SendBufferedText("\n");
		}

		SendBufferedText("EndMessage\n");
	}

	bytecount=m_sendbuffer.size()-startlen;

	return bytecount;
}


const int FCPv2::SendRaw(const char *data, const int datalen)
{
	int bytecount=datalen;

	if(bytecount>0)
	{
		SendBufferedRaw(data,datalen);
	}

	return bytecount;

}

void FCPv2::SocketReceive()
{
	int len=0;

	len=recv(m_serversocket,m_tempbuffer,65535,0);

	if(len>0)
	{

		m_receivebuffer.resize(m_receivebuffer.size()+len);
		std::copy(m_tempbuffer,&m_tempbuffer[len],m_receivebuffer.end()-len);

	}
	// there was an error or server closed connection  - disconnect socket
	else
	{
		Disconnect();
	}
}

void FCPv2::SocketSend()
{
	int len=0;
	if(m_sendbuffer.size()>0)
	{
		len=send(m_serversocket,&m_sendbuffer[0],m_sendbuffer.size(),0);
		if(len>0)
		{
			// move remaining data in buffer to beginning of buffer (erase the bytes we just sent)
			m_sendbuffer.erase(m_sendbuffer.begin(),m_sendbuffer.begin()+len);
		}
		// there was an error with send - disconnect socket
		else
		{
			Disconnect();
		}
	}
}


const bool FCPv2::Update(const long waittime)
{

	if(Connected())
	{
		m_timeval.tv_sec=waittime;
		m_timeval.tv_usec=0;

		FD_ZERO(&m_readfs);
		FD_ZERO(&m_writefs);

		FD_SET(m_serversocket,&m_readfs);
		
		if(m_sendbuffer.size()>0)
		{
			FD_SET(m_serversocket,&m_writefs);
		}

		select(m_serversocket+1,&m_readfs,&m_writefs,0,&m_timeval);

		if(FD_ISSET(m_serversocket,&m_readfs))
		{
			SocketReceive();
		}
		if(Connected() && FD_ISSET(m_serversocket,&m_writefs))
		{
			SocketSend();
		}

		return true;

	}
	else
	{
		return false;
	}

}
