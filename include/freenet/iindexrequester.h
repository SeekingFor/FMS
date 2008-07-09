#ifndef _iindexrequester_
#define _iindexrequester_

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
class IIndexRequester:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IIndexRequester();
	IIndexRequester(FCPv2 *fcp);
	virtual ~IIndexRequester()		{}

	virtual void FCPConnected();
	virtual void FCPDisconnected();
	virtual const bool HandleMessage(FCPMessage &message);

	virtual void Process();

	virtual void RegisterWithThread(FreenetMasterThread *thread);

protected:
	void InitializeIIndexRequester();
	virtual void Initialize()=0;		// initialize m_maxrequests and m_fcpuniquename
	virtual void PopulateIDList()=0;
	virtual void StartRequest(const IDTYPE &id)=0;
	virtual const bool HandleAllData(FCPMessage &message)=0;
	virtual const bool HandleGetFailed(FCPMessage &message)=0;
	virtual void RemoveFromRequestList(const IDTYPE id);

	Poco::DateTime m_tempdate;
	Poco::DateTime m_lastreceived;
	Poco::DateTime m_lastpopulated;
	std::string m_messagebase;
	std::map<IDTYPE,bool> m_ids;			// map of all ids we know and whether we have requested file from them yet
	std::vector<IDTYPE> m_requesting;		// list of ids we are currently requesting from

	// these MUST be populated by child class
	long m_maxrequests;
	std::string m_fcpuniquename;

};

template <class IDTYPE>
IIndexRequester<IDTYPE>::IIndexRequester()
{
	InitializeIIndexRequester();
}

template <class IDTYPE>
IIndexRequester<IDTYPE>::IIndexRequester(FCPv2 *fcp):IFCPConnected(fcp)
{
	InitializeIIndexRequester();
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::FCPConnected()
{
	// make sure variables have been initialized by the derived class
	if(m_maxrequests==-1)
	{
		m_log->fatal("IIndexRequester<IDTYPE>::FCPConnected maxrequests not initialized correctly!");
	}
	if(m_fcpuniquename=="")
	{
		m_log->fatal("IIndexRequester<IDTYPE>::FCPConnected fcpuniquename not initialized correctly!");
	}
	if(m_fcpuniquename.find("|")!=std::string::npos)
	{
		m_log->fatal("IIndexRequester<IDTYPE>::FCPConnected fcpuniquename contains | character!  This is not a valid character!");
	}

	m_lastreceived=Poco::Timestamp();
	m_requesting.clear();
	PopulateIDList();
	m_lastpopulated=Poco::Timestamp();
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::FCPDisconnected()
{
	
}

template <class IDTYPE>
const bool IIndexRequester<IDTYPE>::HandleMessage(FCPMessage &message)
{

	if(message["Identifier"].find(m_fcpuniquename)==0)
	{

		m_lastreceived=Poco::Timestamp();

		if(message.GetName()=="DataFound")
		{
			return true;
		}

		if(message.GetName()=="AllData")
		{
			return HandleAllData(message);
		}

		if(message.GetName()=="GetFailed")
		{
			return HandleGetFailed(message);
		}

		if(message.GetName()=="IdentifierCollision")
		{
			// remove one of the ids from the requesting list
			IDTYPE id;
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);
			StringFunctions::Convert(idparts[1],id);
			RemoveFromRequestList(id);
			return true;
		}
	}

	return false;
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::InitializeIIndexRequester()
{
	m_maxrequests=-1;
	m_fcpuniquename="";

	Option::Instance()->Get("MessageBase",m_messagebase);
	m_tempdate=Poco::Timestamp();
	m_lastreceived=Poco::Timestamp();
	m_lastpopulated=Poco::Timestamp();
	m_lastpopulated-=Poco::Timespan(0,0,10,0,0);
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::Process()
{
	Poco::DateTime now;

	// max is the smaller of the config value or the total number of ids we will request from
	typename std::map<IDTYPE,bool>::size_type max=m_maxrequests>m_ids.size() ? m_ids.size() : m_maxrequests;

	// try to keep up to max requests going
	if(m_requesting.size()<max)
	{
		typename std::map<IDTYPE,bool>::iterator i=m_ids.begin();

		while(i!=m_ids.end() && (*i).second==true)
		{
			i++;
		}

		if(i!=m_ids.end())
		{
			StartRequest((*i).first);
		}
		else
		{
			// we requested from all ids in the list, repopulate the list (only every 10 minutes)
			if(m_lastpopulated<(now-Poco::Timespan(0,0,10,0,0)))
			{
				PopulateIDList();
				m_lastpopulated=Poco::Timestamp();
			}
		}
	}
	// special case - if there were 0 ids on the list when we started then we will never get a chance to repopulate the list
	// this will recheck for ids every minute
	if(m_ids.size()==0 && m_tempdate<(now-Poco::Timespan(0,0,1,0,0)))
	{
		PopulateIDList();
		m_tempdate=now;
	}
	// if we haven't received any messages to this object in 10 minutes, clear the requests and repopulate id list
	if(m_ids.size()>0 && m_lastreceived<(now-Poco::Timespan(0,0,10,0,0)))
	{
		m_log->error("IIndexRequester<IDTYPE>::Process "+m_fcpuniquename+" Object has not received any messages in 10 minutes.  Restarting requests.");
		FCPConnected();
	}

}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::RemoveFromRequestList(const IDTYPE id)
{
	typename std::vector<IDTYPE>::iterator i=m_requesting.begin();
	while(i!=m_requesting.end() && (*i)!=id)
	{
		i++;
	}
	if(i!=m_requesting.end())
	{
		m_requesting.erase(i);
	}
}

#endif	// _iindexrequester_
