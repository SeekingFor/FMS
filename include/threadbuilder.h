#ifndef _threadbuilder_
#define _threadbuilder_

#include "idatabase.h"
#include "ilogger.h"

class ThreadBuilder:public IDatabase,ILogger
{
public:
	ThreadBuilder(SQLite3DB::DB *db):IDatabase(db)			{}

	const bool Build(const long messageid, const long boardid, const bool bydate=false);

private:

};

#endif	// _threadbuilder_
