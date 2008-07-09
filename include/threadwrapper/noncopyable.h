#ifndef _noncopyable_
#define _noncopyable_

class NonCopyable
{
protected:
	NonCopyable()			{}
	virtual ~NonCopyable()	{}

private:
	// restrict copy and assignment
	NonCopyable(const NonCopyable &rhs);
	const NonCopyable &operator=(const NonCopyable &rhs);
};

#endif	// _noncopyable_
