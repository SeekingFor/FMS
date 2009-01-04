#ifndef _ifcpconnected_
#define _ifcpconnected_

#include "fcpv2.h"

/**
	\brief Defines interface for classes that use an existing FCP Connection
*/
class IFCPConnected
{
public:
	IFCPConnected():m_fcp(NULL) {}
	IFCPConnected(FCPv2::Connection *fcp):m_fcp(fcp)	{}
	
	virtual void SetFCPConnection(FCPv2::Connection *fcp)	{ m_fcp=fcp; }
	virtual FCPv2::Connection *GetFCPConnection()			{ return m_fcp; }
	
	/**
		\brief called when the FCP connection becomes disconnected
		
		Parent object is responsible for calling this whenever the FCP connection becomes disconnected
	*/
	virtual void FCPDisconnected()=0;
	/**
		\brief called when the FCP connection becomes connected
		
		Parent object is responsible for calling this whenever the FCP connection is established
	*/
	virtual void FCPConnected()=0;
	
protected:
	FCPv2::Connection *m_fcp;
};

#endif	// _ifcpconnected_
