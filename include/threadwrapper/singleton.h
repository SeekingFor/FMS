#ifndef _singleton_
#define _singleton_

#include <cassert>

#include "noncopyable.h"

//! SingletonDestroyer
template <class T>
class Destroyer
{
	T* doomed;
	public:
  
	Destroyer(T* q) : doomed(q) { assert(doomed); }
	~Destroyer();
};

template <class T>
Destroyer<T>::~Destroyer()
{
	try
	{
		if(doomed)
		{
			delete doomed;
		}
	}
	catch(...)
	{
    }
  
	doomed = 0;
}

template <class T>
class Singleton:public NonCopyable
{
public:

	static T *Instance();

private:
	
};

template <class T>
T *Singleton<T>::Instance()
{
	static T *obj=0;
	static Poco::FastMutex mutex;
	Poco::ScopedLock<Poco::FastMutex> g(mutex);
	if(obj==0)
	{
		obj=new T();
		static Destroyer<T> des(obj);
	}
	return const_cast<T*>(obj);
}

#endif	// _singleton_
