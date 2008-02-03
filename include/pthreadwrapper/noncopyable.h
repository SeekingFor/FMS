#ifndef _pthread_noncopyable_
#define _pthread_noncopyable_

namespace PThread
{

class NonCopyable
{
protected:
	NonCopyable()	{}
	~NonCopyable()	{}

private:
	// restrict copy and assignment
	NonCopyable(const NonCopyable &rhs);
	const NonCopyable &operator=(const NonCopyable &rhs);
};
	
}	// namespace

#endif	// _pthread_noncopyable_
