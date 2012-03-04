#ifndef _socket_defines_
#define _socket_defines_

#ifdef _WIN32
	#include <winsock2.h>
#endif

#include <string>

#ifndef SOCKET
#ifndef _WIN32
	typedef unsigned int SOCKET;
#endif
#endif

#ifndef INVALID_SOCKET
	#define INVALID_SOCKET (SOCKET)(~0)
#endif

#ifndef SOCKET_ERROR
	#define SOCKET_ERROR (-1)
#endif

std::string GetSocketErrorMessage();
int GetSocketErrorNumber();

#endif	// _socket_defines_
