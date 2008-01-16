#include "../../include/freenet/introductionpuzzleremover.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IntroductionPuzzleRemover::IntroductionPuzzleRemover()
{
	m_lastchecked.SetToGMTime();
}

void IntroductionPuzzleRemover::Process()
{
	DateTime now;
	DateTime date;
	now.SetToGMTime();
	date.SetToGMTime();

	// check once a day
	if(m_lastchecked<(now-1.0))
	{

		date.Add(0,0,0,-2);

		// delete all puzzles 2 or more days old
		m_db->Execute("DELETE FROM tblIntroductionPuzzleInserts WHERE Day<='"+date.Format("%Y-%m-%d")+"';");
		m_db->Execute("DELETE FROM tblIntroductionPuzzleRequests WHERE Day<='"+date.Format("%Y-%m-%d")+"';");

		m_lastchecked=now;
	}
}

void IntroductionPuzzleRemover::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterPeriodicProcessor(this);
}
