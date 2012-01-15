#ifndef _soneactiverequester_
#define _soneactiverequester_

#include "sonerequester.h"

class SoneActiveRequester:public SoneRequester
{
public:
	SoneActiveRequester(SQLite3DB::DB *db);
	SoneActiveRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();

};

#endif	// _soneactiverequester_
