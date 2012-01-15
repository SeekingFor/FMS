#ifndef _message_
#define _message_

#include "idatabase.h"
#include "ilogger.h"

#include <Poco/DateTime.h>

class Message:public IDatabase,public ILogger
{
public:
	Message(SQLite3DB::DB *db);
	Message(SQLite3DB::DB *db, const long dbmessageid, const long boardid);
	//Message(SQLite3DB::DB *db, const std::string &messageuuid);

	const long GetDBMessageID() const				{ return m_dbmessageid; }
	const std::string GetMessageUUID() const		{ return m_messageuuid; }
	const std::string GetSubject() const			{ return m_subject; }
	const std::string GetBody() const				{ return m_body; }
	const std::string GetReplyBoardName()			{ return m_replyboardname; }
	const Poco::DateTime GetDateTime() const		{ return m_datetime; }
	const std::string GetFromName() const			{ return m_fromname; }
	std::vector<std::string> GetBoards() const		{ return m_boards; }
	std::map<long,std::string> GetInReplyTo() const	{ return m_inreplyto; }

	void SetFromName(const std::string &fromname)	{ m_fromname=fromname; }

	void AddInsertFileAttachment(const std::string &filename, const std::string &mimetype, const std::vector<unsigned char> &data);

	const std::string GetNNTPHeaders() const;
	const std::string GetNNTPArticleID() const;
	const std::string GetNNTPBody() const;
	const long GetNNTPMessageID() const				{ return m_nntpmessageid; }

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

	const bool LoadDB(const long dbmessageid, const long boardid=-1);
	const bool LoadNNTP(const long nntpmessageid, const long boardid);
	const bool LoadNextNNTP(const long nntpmessageid, const long boardid);		// loads next message in board with messageid > passed id
	const bool LoadPreviousNNTP(const long nntpmessageid, const long boardid);	// loads previous message in board with messageid < passed id
	const bool Load(const std::string &messageuuid);
	
	const bool ParseNNTPMessage(const std::string &nntpmessage);
	const bool Create(const long localidentityid, const long boardid, const std::string &subject, const std::string &body, const std::vector<std::string> &references);

	const bool PostedToAdministrationBoard()		{ return CheckForAdministrationBoard(m_boards); }

	const bool PrepareFreenetInsert();
	const bool StartFreenetInsert();
	void HandleAdministrationMessage();

	const std::string GetMessageXML(const bool withfakeattachmentkeys=false) const;

	static const long MaxMessageXMLSize()	{ return 1000000; }
	static const long LineMaxBytes(const std::string &body);

	enum messagesource
	{
		SOURCE_FMS=1,
		SOURCE_AUTOMATED=2,
		SOURCE_FROST=3,
		SOURCE_SONE=4
	};

private:
	void Initialize();
	// checks vector of boards for any special administration boards - if it finds one true is returned, otherwise false
	const bool CheckForAdministrationBoard(const std::vector<std::string> &boards);
	void HandleChangeTrust();
	void StripAdministrationBoards();	// removes administration boards from boards vector
	const int FindLocalIdentityID(const std::string &name);
	const std::string SanitizeFromName(const std::string &fromname) const;

	struct insertfileattachment
	{
		insertfileattachment(const std::string &filename, const std::string &mimetype, const std::vector<unsigned char> &data):m_filename(filename),m_mimetype(mimetype),m_data(data)	{}
		std::string m_filename;
		std::string m_mimetype;
		std::vector<unsigned char> m_data;
	};

	struct receivedfileattachment
	{
		receivedfileattachment(const std::string &key, const int size):m_key(key),m_size(size)	{}
		std::string m_key;
		int m_size;
	};

	bool m_uniqueboardmessageids;
	long m_dbmessageid;
	long m_nntpmessageid;
	bool m_addnewpostfromidentities;
	std::string m_messageuuid;
	std::string m_subject;
	std::string m_body;
	std::string m_replyboardname;
	Poco::DateTime m_datetime;
	std::string m_fromname;
	std::vector<std::string> m_boards;
	std::map<long,std::string> m_inreplyto;
	std::vector<insertfileattachment> m_insertfileattachments;
	std::vector<receivedfileattachment> m_receivedfileattachments;
	long m_changemessagetrustonreply;
	long m_minlocalmessagetrust;
	long m_minlocaltrustlisttrust;
	int m_bodylinemaxbytes;

};

#endif	// _message_
