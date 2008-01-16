#ifndef _ilogger_
#define _ilogger_

#include "logfile.h"

/**
	\brief Base class for classes that want to use the singleton LogFile object
*/
class ILogger
{
public:
	ILogger():m_log(LogFile::instance()) {}
	
protected:
	LogFile *m_log;
};

#endif	// _ilogger_
