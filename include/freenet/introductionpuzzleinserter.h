#ifndef _introductionpuzzle_inserter_
#define _introductionpuzzle_inserter_

#include "iindexinserter.h"

#include <map>
#include <Poco/DateTime.h>

class IntroductionPuzzleInserter:public IIndexInserter<long>
{
public:
	IntroductionPuzzleInserter();
	IntroductionPuzzleInserter(FCPv2 *fcp);

private:
	void Initialize();
	void CheckForNeededInsert();
	const bool StartInsert(const long &localidentityid);
	void GenerateCaptcha(std::string &encodeddata, std::string &solution);
	const bool HandlePutSuccessful(FCPMessage &message);
	const bool HandlePutFailed(FCPMessage &message);

	Poco::DateTime m_lastchecked;
	int m_maxpuzzleinserts;
	std::map<int,Poco::DateTime> m_lastinserted;

};

#endif	// _introductionpuzzle_inserter_
