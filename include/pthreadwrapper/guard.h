#ifndef _pthread_guard_
#define _pthread_guard_

namespace PThread
{

class Mutex;

class Guard
{
public:
	Guard();
	Guard(Mutex &mutex);
	~Guard();
private:
	Mutex *m_mutex;
};

}	// namespace

#endif	// _pthread_guard_
