#ifndef _ilogger_
#define _ilogger_

#include <Poco/Util/ServerApplication.h>
#include <Poco/LogFile.h>

/**
	\brief Base class for classes that want to use the singleton LogFile object
*/
class ILogger
{
public:
	ILogger():m_log(&Poco::Util::ServerApplication::instance().logger()) {}
	
protected:
	Poco::Logger *m_log;
};

#endif	// _ilogger_
