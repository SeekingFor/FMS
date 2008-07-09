#ifndef _iindexinserter_
#define _iindexinserter_

#include "../idatabase.h"
#include "../ilogger.h"
#include "../option.h"
#include "../stringfunctions.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

#include <Poco/DateTime.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>

#ifdef XMEM
	#include <xmem.h>
#endif

template <class IDTYPE>
class IIndexInserter:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IIndexInserter();
	IIndexInserter(FCPv2 *fcp);
	virtual ~IIndexInserter()		{}

	virtual void FCPConnected();
	virtual void FCPDisconnected();
	virtual const bool HandleMessage(FCPMessage &message);

	virtual void Process();

	virtual void RegisterWithThread(FreenetMasterThread *thread);

protected:
	void InitializeIIndexInserter();
	virtual void Initialize()=0;		// initialize m_fcpuniquename
	virtual const bool HandlePutSuccessful(FCPMessage &message)=0;
	virtual const bool HandlePutFailed(FCPMessage &message)=0;
	virtual const bool StartInsert(const IDTYPE &id)=0;
	virtual void CheckForNeededInsert()=0;
	virtual void RemoveFromInsertList(const IDTYPE id);

	std::vector<IDTYPE> m_inserting;		// list of ids we are inserting
	std::string m_messagebase;
	Poco::DateTime m_lastchecked;

	// these MUST be populated by child class
	std::string m_fcpuniquename;
};

template <class IDTYPE>
IIndexInserter<IDTYPE>::IIndexInserter()
{
	InitializeIIndexInserter();
}

template <class IDTYPE>
IIndexInserter<IDTYPE>::IIndexInserter(FCPv2 *fcp):IFCPConnected(fcp)
{
	InitializeIIndexInserter();
}

template <class IDTYPE>
void IIndexInserter<IDTYPE>::FCPConnected()
{
	// make sure variables have been initialized by the derived class
	if(m_fcpuniquename=="")
	{
		m_log->fatal("IIndexInserter<IDTYPE>::FCPConnected fcpuniquename not initialized correctly!");
	}
	if(m_fcpuniquename.find("|")!=std::string::npos)
	{
		m_log->fatal("IIndexInserter<IDTYPE>::FCPConnected fcpuniquename contains | character!  This is not a valid character!");
	}

	m_inserting.clear();
}

template <class IDTYPE>
void IIndexInserter<IDTYPE>::FCPDisconnected()
{
	
}

template <class IDTYPE>
const bool IIndexInserter<IDTYPE>::HandleMessage(FCPMessage &message)
{

	if(message["Identifier"].find(m_fcpuniquename)==0)
	{
		if(message.GetName()=="URIGenerated")
		{
			return true;
		}

		if(message.GetName()=="PutSuccessful")
		{
			return HandlePutSuccessful(message);
		}

		if(message.GetName()=="PutFailed")
		{
			return HandlePutFailed(message);
		}

		if(message.GetName()=="IdentifierCollision")
		{
			// remove one of the ids from the requesting list
			IDTYPE id;
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);
			StringFunctions::Convert(idparts[1],id);
			RemoveFromInsertList(id);
			m_log->debug("IIndexInserter<IDTYPE>::HandleMessage IdentifierCollision for "+m_fcpuniquename+" "+message["Identifier"]);
			return true;
		}
	}

	return false;
}

template <class IDTYPE>
void IIndexInserter<IDTYPE>::InitializeIIndexInserter()
{
	m_fcpuniquename="";
	Option::Instance()->Get("MessageBase",m_messagebase);
	m_lastchecked=Poco::Timestamp();
}

template <class IDTYPE>
void IIndexInserter<IDTYPE>::Process()
{
	Poco::DateTime now;

	if(m_lastchecked<(now-Poco::Timespan(0,0,1,0,0)))
	{
		CheckForNeededInsert();
		m_lastchecked=now;
	}
}

template <class IDTYPE>
void IIndexInserter<IDTYPE>::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

template <class IDTYPE>
void IIndexInserter<IDTYPE>::RemoveFromInsertList(const IDTYPE identityid)
{
	typename std::vector<IDTYPE>::iterator i=m_inserting.begin();
	while(i!=m_inserting.end() && (*i)!=identityid)
	{
		i++;
	}
	if(i!=m_inserting.end())
	{
		m_inserting.erase(i);
	}
}

#endif	// _iindexrequester_
