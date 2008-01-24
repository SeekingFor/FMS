#ifndef _message_
#define _message_

#include "idatabase.h"
#include "ilogger.h"
#include "datetime.h"

class Message:public IDatabase,public ILogger
{
public:
	Message();
	Message(const long messageid);
	Message(const std::string &messageuuid);

	const long GetMessageID() const					{ return m_messageid; }
	const std::string GetMessageUUID() const		{ return m_messageuuid; }
	const std::string GetSubject() const			{ return m_subject; }
	const std::string GetBody() const				{ return m_body; }
	const std::string GetReplyBoardName()			{ return m_replyboardname; }
	const DateTime GetDateTime() const				{ return m_datetime; }
	const std::string GetFromName() const			{ return m_fromname; }
	std::vector<std::string> GetBoards() const		{ return m_boards; }
	std::map<long,std::string> GetInReplyTo() const	{ return m_inreplyto; }

	const std::string GetNNTPHeaders() const;
	const std::string GetNNTPArticleID() const;
	const std::string GetNNTPBody() const;

/*
	void SetMessageUUID(const std::string &messageuuid)					{ m_messageuuid=messageuuid; }
	void SetSubject(const std::string &subject)							{ m_subject=subject; }
	void SetBody(const std::string &body)								{ m_body=body; }
	void SetReplyBoardName(const std::string &replyboardname)			{ m_replyboardname=replyboardname; }
	void SetDateTime(const DateTime &datetime)							{ m_datetime=datetime; }
	void SetFromName(const std::string &fromname)						{ m_fromname=fromname; }
	void AddBoard(const std::string &board)								{ m_boards.push_back(board); }
	void AddInReplyTo(const long index, const std::string &messageid)	{ m_inreplyto[index]=messageid; }
*/

	const bool Load(const long messageid, const long boardid=-1);
	const bool LoadNext(const long messageid, const long boardid=-1);		// loads next message in board with messageid > passed id
	const bool LoadPrevious(const long messageid, const long boardid=-1);	// loads previous message in board with messageid < passed id
	const bool Load(const std::string &messageuuid);
	
	const bool ParseNNTPMessage(const std::string &nntpmessage);

	void StartFreenetInsert();

private:
	void Initialize();

	long m_messageid;
	std::string m_messageuuid;
	std::string m_subject;
	std::string m_body;
	std::string m_replyboardname;
	DateTime m_datetime;
	std::string m_fromname;
	std::vector<std::string> m_boards;
	std::map<long,std::string> m_inreplyto;

};

#endif	// _message_
