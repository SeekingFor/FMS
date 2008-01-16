#include "../include/uuidgenerator.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string UUIDGenerator::Generate()
{

	return RandHex(8)+"-"+RandHex(4)+"-4"+RandHex(3)+"-"+RandHex(4)+"-"+RandHex(12);

}

const std::string UUIDGenerator::RandHex(const int len)
{
	static std::string hexchars="0123456789ABCDEF";
	std::string rval="";
	for(int i=0; i<len; i++)
	{
		rval+=hexchars[rand()%hexchars.size()];
	}
	return rval;
}