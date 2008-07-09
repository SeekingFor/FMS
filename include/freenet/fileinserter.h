#ifndef _fileinserter_
#define _fileinserter_

#include "iindexinserter.h"

class FileInserter:public IIndexInserter<long>
{
public:
	FileInserter();
	FileInserter(FCPv2 *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPMessage &message);
	const bool HandlePutFailed(FCPMessage &message);
	const bool StartInsert(const long &fileinsertid);
	void CheckForNeededInsert();

};

#endif	// _fileinserter_
