#ifndef _pthread_wrapper_
#define _pthread_wrapper_

#include <exception>

namespace PThread
{
	
class DeadlockException:public std::exception
{
public:
	const char *what() const throw()	{ return "Deadlock Exception"; }
};
	
}	// namespace

#endif	// _pthread_wrapper_
