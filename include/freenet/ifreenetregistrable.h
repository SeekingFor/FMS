#ifndef _freenet_registrable_
#define _freenet_registrable_

#include "freenetmasterthread.h"

// forward declaration
class FreenetMasterThread;

class IFreenetRegistrable
{
public:
	IFreenetRegistrable()			{}
	virtual ~IFreenetRegistrable()	{}

	virtual void RegisterWithThread(FreenetMasterThread *thread)=0;
	
};

#endif	// _freenet_registrable_
