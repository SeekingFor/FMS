#ifndef _fileinserter_
#define _fileinserter_

#include "iindexinserter.h"

class FileInserter:public IIndexInserter<long>
{
public:
	FileInserter();
	FileInserter(FCPv2::Connection *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPv2::Message &message);
	const bool HandlePutFailed(FCPv2::Message &message);
	const bool StartInsert(const long &fileinsertid);
	void CheckForNeededInsert();

};

#endif	// _fileinserter_
