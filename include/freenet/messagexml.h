#ifndef _messagexml_
#define _messagexml_

#include <vector>
#include <map>

#include "../ifmsxmldocument.h"

class MessageXML:public IFMSXMLDocument
{
public:

	MessageXML();

	virtual std::string GetXML();
	virtual const bool ParseXML(const std::string &xml);

	struct fileattachment
	{
		fileattachment(const std::string &key, const int size):m_key(key),m_size(size)	{}
		std::string m_key;
		int m_size;
	};

	const std::string GetDate() const								{ return m_date; }
	const std::string GetTime() const								{ return m_time; }
	const std::string GetSubject() const							{ return m_subject; }
	const std::string GetMessageID() const							{ return m_messageid; }
	const std::string GetReplyBoard() const							{ return m_replyboard; }
	const std::string GetBody() const								{ return m_body; }
	const std::vector<std::string> GetBoards() const				{ return m_boards; }
	const std::map<long,std::string> GetInReplyTo() const			{ return m_inreplyto; }
	const std::vector<fileattachment> GetFileAttachments() const	{ return m_fileattachments; }
	const std::string GetLastError() const							{ return m_lasterror; }

	void SetDate(const std::string &date)								{ m_date=date; }
	void SetTime(const std::string &time)								{ m_time=time; }
	void SetSubject(const std::string &subject)							{ m_subject=subject; }
	void SetMessageID(const std::string &messageid)						{ m_messageid=messageid; }
	void SetReplyBoard(const std::string &replyboard)					{ m_replyboard=replyboard; }
	void SetBody(const std::string &body)								{ m_body=body; }
	void AddBoard(const std::string &board)								{ m_boards.push_back(board); }
	void AddInReplyTo(const long index, const std::string &messageid)	{ m_inreplyto[index]=messageid; }
	void AddFileAttachment(const std::string &key, const int size)		{ m_fileattachments.push_back(fileattachment(key,size)); }

protected:
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
	std::string m_lasterror;

};

#endif	// _messagexml_
