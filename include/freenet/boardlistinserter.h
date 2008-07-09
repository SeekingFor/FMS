#ifndef _boardlistinserter_
#define _boardlistinserter_

#include "iindexinserter.h"

class BoardListInserter:public IIndexInserter<long>
{
public:
	BoardListInserter();
	BoardListInserter(FCPv2 *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPMessage &message);
	const bool HandlePutFailed(FCPMessage &message);
	const bool StartInsert(const long &localidentityid);
	void CheckForNeededInsert();
};

#endif	// _boardlistinserter_
