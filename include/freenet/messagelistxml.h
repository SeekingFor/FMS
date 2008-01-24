#ifndef _messagelistxml_
#define _messagelistxml_

#include "../ifmsxmldocument.h"

#include <vector>

class MessageListXML:public IFMSXMLDocument
{
public:
	MessageListXML();

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	void ClearMessages()	{ m_messages.clear(); }

	void AddMessage(const std::string &date, const long index, const std::vector<std::string> boards);

	const long MessageCount()	{ return m_messages.size(); }
	std::string GetDate(const long index);
	const long GetIndex(const long index);
	std::vector<std::string> GetBoards(const long index);

private:
	struct message
	{
		message(const std::string &date, const long index, const std::vector<std::string> &boards):m_date(date),m_index(index),m_boards(boards)	{}
		std::string m_date;
		long m_index;
		std::vector<std::string> m_boards;
	};

	void Initialize();

	std::vector<message> m_messages;

};

#endif	// _messagelistxml_
