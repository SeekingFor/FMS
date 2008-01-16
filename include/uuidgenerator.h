#ifndef _uuidgenerator_
#define _uuidgenerator_

#include <string>

/**
	\brief UUID v4 (based on random numbers)

	reference : http://lists.evolt.org/pipermail/javascript/2006-July/010716.html
*/
class UUIDGenerator
{
public:

	const std::string Generate();

private:

	const std::string RandHex(const int len);

};

#endif	// _uuidgenerator_
