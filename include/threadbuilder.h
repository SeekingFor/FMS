#ifndef _threadbuilder_
#define _threadbuilder_

#include "idatabase.h"

class ThreadBuilder:public IDatabase
{
public:

	const bool Build(const long messageid, const long boardid, const bool bydate=false);

private:

};

#endif	// _threadbuilder_
