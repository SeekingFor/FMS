#ifndef _messagelistxml_
#define _messagelistxml_

#include "../ifmsxmldocument.h"

#include <vector>

class MessageListXML:public IFMSXMLDocument
{
private:
	struct message
	{
		message(const std::string &date, const long index, const std::vector<std::string> &boards):m_date(date),m_index(index),m_boards(boards)	{}
		std::string m_date;
		long m_index;
		std::vector<std::string> m_boards;
	};
	struct externalmessage
	{
		externalmessage(const std::string &type, const std::string identity, const std::string &date, const long index, const std::vector<std::string> &boards):m_type(type),m_identity(identity),m_date(date),m_index(index),m_boards(boards)	{}
		externalmessage(const std::string &type, const std::string messagekey, const std::string &date, const std::vector<std::string> &boards):m_type(type),m_messagekey(messagekey),m_date(date),m_boards(boards)								{}
		std::string m_type;
		std::string m_identity;
		std::string m_messagekey;
		long m_index;
		std::string m_date;
		std::vector<std::string> m_boards;
	};
public:
	MessageListXML();

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	void ClearMessages()	{ m_messages.clear(); }

	void AddMessage(const std::string &date, const long index, const std::vector<std::string> &boards);
	void AddExternalMessage(const std::string &identity, const std::string &date, const long index, const std::vector<std::string> &boards);
	void AddExternalMessage(const std::string &messagekey, const std::string &date, const std::vector<std::string> &boards);

	const std::vector<message>::size_type MessageCount()	{ return m_messages.size(); }
	std::string GetDate(const long index);
	const long GetIndex(const long index);
	std::vector<std::string> GetBoards(const long index);

	const std::vector<externalmessage>::size_type ExternalMessageCount()	{ return m_externalmessages.size(); }
	std::string GetExternalType(const long index);
	std::string GetExternalIdentity(const long index);
	std::string GetExternalMessageKey(const long index);
	const long GetExternalIndex(const long index);
	std::string GetExternalDate(const long index);
	std::vector<std::string> GetExternalBoards(const long index);

private:
	void Initialize();

	std::vector<message> m_messages;
	std::vector<externalmessage> m_externalmessages;

};

#endif	// _messagelistxml_
