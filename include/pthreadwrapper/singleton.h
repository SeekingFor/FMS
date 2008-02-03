#ifndef _pthread_singleton_
#define _pthread_singleton_

#include "noncopyable.h"
#include "guard.h"

#include <cassert>

namespace PThread
{

//! SingletonDestroyer
template <class T>
class Destroyer {
  
  T* doomed;
  
 public:
  
  Destroyer(T* q) : doomed(q) {
    assert(doomed);
  }
  
  ~Destroyer();

};

template <class T>
Destroyer<T>::~Destroyer() {
  
  try {
    
    if(doomed)
      delete doomed;
    
  } catch(...) { }
  
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
	static Mutex mutex;
	Guard g(mutex);
	if(obj==0)
	{
		obj=new T();
		static Destroyer<T> des(obj);
	}
	return const_cast<T*>(obj);
}

}	// namespace

#endif	// _pthread_singleton_
