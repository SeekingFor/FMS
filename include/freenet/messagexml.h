#ifndef _messagexml_
#define _messagexml_

#include <vector>
#include <map>

#include "../ifmsxmldocument.h"

class MessageXML:public IFMSXMLDocument
{
public:

	MessageXML();

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	struct fileattachment
	{
		fileattachment(const std::string &key, const int size):m_key(key),m_size(size)	{}
		std::string m_key;
		int m_size;
	};

	std::string GetDate()								{ return m_date; }
	std::string GetTime()								{ return m_time; }
	std::string GetSubject()							{ return m_subject; }
	std::string GetMessageID()							{ return m_messageid; }
	std::string GetReplyBoard()							{ return m_replyboard; }
	std::string GetBody()								{ return m_body; }
	std::vector<std::string> GetBoards()				{ return m_boards; }
	std::map<long,std::string> GetInReplyTo()			{ return m_inreplyto; }
	std::vector<fileattachment> GetFileAttachments()	{ return m_fileattachments; }

	void SetDate(const std::string &date)								{ m_date=date; }
	void SetTime(const std::string &time)								{ m_time=time; }
	void SetSubject(const std::string &subject)							{ m_subject=subject; }
	void SetMessageID(const std::string &messageid)						{ m_messageid=messageid; }
	void SetReplyBoard(const std::string &replyboard)					{ m_replyboard=replyboard; }
	void SetBody(const std::string &body)								{ m_body=body; }
	void AddBoard(const std::string &board)								{ m_boards.push_back(board); }
	void AddInReplyTo(const long index, const std::string &messageid)	{ m_inreplyto[index]=messageid; }
	void AddFileAttachment(const std::string &key, const int size)		{ m_fileattachments.push_back(fileattachment(key,size)); }

private:
	void Initialize();

	std::string m_date;
	std::string m_time;
	std::string m_subject;
	std::string m_messageid;
	std::vector<std::string> m_boards;
	std::string m_replyboard;
	std::map<long,std::string> m_inreplyto;
	std::vector<fileattachment> m_fileattachments;
	std::string m_body;

};

#endif	// _messagexml_
