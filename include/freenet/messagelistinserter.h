#ifndef _messagelistinserter_
#define _messagelistinserter_

#include "iindexinserter.h"

class MessageListInserter:public IIndexInserter<long>
{
public:
	MessageListInserter(SQLite3DB::DB *db);
	MessageListInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPv2::Message &message);
	const bool HandlePutFailed(FCPv2::Message &message);
	const bool HandleSimpleProgress(FCPv2::Message &message);
	const bool StartInsert(const long &localidentityid);
	void CheckForNeededInsert();

	long m_daysbackward;
	std::map<long,std::string> m_lastinsertedxml;	// last xml document inserted for each local identity
	Poco::DateTime m_laststartedinsert;

};

#endif	// _messagelistinserter_
