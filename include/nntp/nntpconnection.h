#ifndef _nntpconnection_
#define _nntpconnection_

#include "../socketdefines.h"
#include "../ilogger.h"
#include "../message.h"

#include <string>
#include <vector>
#include <zthread/Runnable.h>

#ifdef _WIN32

#else
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

class NNTPConnection:public ZThread::Runnable,public ILogger//,public IDatabase
{
public:
	NNTPConnection(SOCKET sock);
	~NNTPConnection();

	void Disconnect();
	const bool Disconnected()		{ return m_socket==INVALID_SOCKET; }

	void run();

private:

	typedef enum ClientMode
	{
		MODE_NONE=0,
		MODE_READER
	};
	struct NNTPCommand
	{
		std::string m_command;
		std::vector<std::string> m_arguments;
	};
	struct ClientStatus
	{
		ClientMode m_mode;
		bool m_allowpost;
		bool m_isposting;
		long m_boardid;
		long m_messageid;
	};

	void SendBuffered(const std::string &data);
	void SendBufferedLine(const std::string &data)	{ SendBuffered(data+"\r\n"); }
	void SocketSend();			// immediately send buffered data - will block if send if no ready
	void SocketReceive();		// immediately recv data on socket - will block if no data is waiting
	void HandleReceivedData();
	std::vector<char>::iterator Find(std::vector<char> &buffer, const std::string &val);
	const bool HandleCommand(const NNTPCommand &command);
	void HandlePostedMessage(const std::string &message);

	void SendArticleParts(const NNTPCommand &command);
	void SendArticleOverInfo(Message &message);

	// various NNTP commands to handle
	const bool HandleQuitCommand(const NNTPCommand &command);
	const bool HandleModeCommand(const NNTPCommand &command);
	const bool HandleCapabilitiesCommand(const NNTPCommand &command);
	const bool HandleHelpCommand(const NNTPCommand &command);
	const bool HandleDateCommand(const NNTPCommand &command);
	const bool HandleListCommand(const NNTPCommand &command);
	const bool HandleGroupCommand(const NNTPCommand &command);
	const bool HandleListGroupCommand(const NNTPCommand &command);
	const bool HandleNextCommand(const NNTPCommand &command);
	const bool HandleLastCommand(const NNTPCommand &command);
	const bool HandleArticleCommand(const NNTPCommand &command);
	const bool HandleHeadCommand(const NNTPCommand &command);
	const bool HandleBodyCommand(const NNTPCommand &command);
	const bool HandleStatCommand(const NNTPCommand &command);
	const bool HandleNewGroupsCommand(const NNTPCommand &command);
	const bool HandlePostCommand(const NNTPCommand &command);
	const bool HandleOverCommand(const NNTPCommand &command);

	SOCKET m_socket;
	ClientStatus m_status;
	std::vector<char> m_sendbuffer;
	std::vector<char> m_receivebuffer;
	std::vector<char> m_tempbuffer;

};

#endif	// _nntpconnection_
