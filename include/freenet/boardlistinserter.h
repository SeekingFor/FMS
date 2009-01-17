#ifndef _boardlistinserter_
#define _boardlistinserter_

#include "iindexinserter.h"

class BoardListInserter:public IIndexInserter<long>
{
public:
	BoardListInserter(SQLite3DB::DB *db);
	BoardListInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPv2::Message &message);
	const bool HandlePutFailed(FCPv2::Message &message);
	const bool StartInsert(const long &localidentityid);
	void CheckForNeededInsert();
};

#endif	// _boardlistinserter_
