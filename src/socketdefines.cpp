#include "../include/socketdefines.h"
#include <cerrno>
#include <cstring>

#ifdef XMEM
	#include <xmem.h>
#endif

std::string GetSocketErrorMessage()
{
	if(strerror(GetSocketErrorNumber()))
	{
		return std::string(strerror(GetSocketErrorNumber()));
	}
	else
	{
		return std::string("");	
	}
}

int GetSocketErrorNumber()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}
