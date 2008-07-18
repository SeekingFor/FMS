#ifndef _global_
#define _global_

#include <string>
#include <Poco/ScopedLock.h>
#include <Poco/Mutex.h>

#define VERSION_MAJOR		"0"
#define VERSION_MINOR		"3"
#define VERSION_RELEASE		"9"
#define FMS_VERSION			VERSION_MAJOR"."VERSION_MINOR"."VERSION_RELEASE

typedef Poco::ScopedLock<Poco::FastMutex> Guard;

std::string CreateShortIdentityName(const std::string &name, const std::string &publickey);

#endif	// _global_
