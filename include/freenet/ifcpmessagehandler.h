#ifndef _ifcpmessagehandler_
#define _ifcpmessagehandler_

#include "fcpv2.h"

/**
	\brief Defines interface for classes that handle FCP messages
*/
class IFCPMessageHandler
{
public:
	/**
		\brief Handles an FCP message
		\param message FCP message to handle
		\return true if the message was handled, false if it was not
	*/
	virtual const bool HandleMessage(FCPv2::Message &message)=0;
};

#endif	// _ifcpmessagehandler_
