#ifndef _iperiodicprocessor_
#define _iperiodicprocessor_

/**
	\brief Defines interface for classes that are periodically processed
*/
class IPeriodicProcessor
{
public:
	/**
		\brief Lets class do any needed processing
	*/
	virtual void Process()=0;
};

#endif	// _iperiodicprocessor_
