#ifndef _introductionpuzzleremover_
#define _introductionpuzzleremover_

#include "../idatabase.h"
#include "../ilogger.h"
#include "../datetime.h"
#include "ifreenetregistrable.h"
#include "iperiodicprocessor.h"

/**
	\brief Removes stale IntroductionPuzzles from database
*/
class IntroductionPuzzleRemover:public IFreenetRegistrable,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IntroductionPuzzleRemover();

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);

private:

	DateTime m_lastchecked;

};

#endif	// _introductionpuzzleremover_
