/*

	FCPv2 C++ library
	
	link with ws2_32.lib in Windows

*/

#ifndef _fcpv2_
#define _fcpv2_

#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include <string>
#include <vector>
#include <map>


class FCPMessage:public std::map<std::string, std::string >
{
public:
	FCPMessage() {};
	FCPMessage(const std::string &name) {m_name=name;}

	const std::string GetName() const { return m_name; }
	void SetName(const std::string &name) { m_name=name; }
	
	void Reset() { m_name=""; clear(); }

protected:
	std::string m_name;
};

class FCPv2
{
public:
	FCPv2();
	~FCPv2();

	const bool Connect(const char *host, const int port);
	const bool Disconnect();

	const bool Connected() const { return m_serversocket!=-1 ? true : false ; }

	const bool Update(const long waittime);

	const int SendMessage(const char *messagename, const int fieldcount, ...);
	const int SendMessage(FCPMessage &message);
	const int SendRaw(const char *data, const int datalen);
	const std::vector<char>::size_type SendBufferSize()	const { return m_sendbuffer.size(); }

	FCPMessage ReceiveMessage();
	const long ReceiveRaw(char *data, long &datalen);	// data must be preallocated, with datalen being max length of data.  Returns length of data received
	const std::vector<char>::size_type ReceiveBufferSize() const { return m_receivebuffer.size(); }

private:
	
	void SocketReceive();
	void SocketSend();

	void SendBufferedText(const char *text);		// puts text on send buffer
	void SendBufferedRaw(const char *data, const long len);	// puts raw data on send buffer

	int FindOnReceiveBuffer(const char *text);		// finds text string on receive buffer and returns index to first char position, -1 if not found


#ifdef _WIN32
	static bool m_wsastartup;
#endif

	int m_serversocket;

	char *m_tempbuffer;			// temp buffer used for recv

	std::vector<char> m_sendbuffer;
	std::vector<char> m_receivebuffer;

	fd_set m_readfs;
	fd_set m_writefs;
	struct timeval m_timeval;

};

#endif	// _fcpv2_
