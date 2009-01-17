#ifndef _messageinserter_
#define _messageinserter_

#include "iindexinserter.h"

// only handle 1 insert at a time
class MessageInserter:public IIndexInserter<std::string>
{
public:
	MessageInserter(SQLite3DB::DB *db);
	MessageInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPv2::Message &message);
	const bool HandlePutFailed(FCPv2::Message &message);
	const bool StartInsert(const std::string &messageuuid);
	void CheckForNeededInsert();

};

#endif	// _messageinserter_
